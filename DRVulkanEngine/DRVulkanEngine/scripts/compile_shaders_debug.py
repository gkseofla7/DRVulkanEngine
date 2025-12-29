import os
import time
import subprocess
import sys
import argparse
from pathlib import Path
from datetime import datetime
from watchdog.observers import Observer
from watchdog.events import FileSystemEventHandler

# 콘솔 출력 컬러 코드
class Colors:
    HEADER = '\033[95m'
    OKBLUE = '\033[94m'
    OKGREEN = '\033[92m'
    WARNING = '\033[93m'
    FAIL = '\033[91m'
    ENDC = '\033[0m'
    BOLD = '\033[1m'

class ShaderCompilerHandler(FileSystemEventHandler):
    def __init__(self, watch_dir, output_dir, debug=False, optimize=False):
        self.watch_dir = Path(watch_dir)
        self.output_dir = Path(output_dir) if output_dir else self.watch_dir
        self.debug_mode = debug
        self.optimize_mode = optimize
        
        self.shader_extensions = {'.vert', '.frag', '.geom', '.tesc', '.tese', '.comp'}
        # 매뉴얼 권장 사항인 glslangValidator를 찾습니다.
        self.compiler_path = self._find_compiler()

        print(f"{Colors.HEADER}--- Vulkan 1.3 Pro Shader Watcher ---{Colors.ENDC}")
        print(f"Monitoring: {Colors.OKBLUE}{self.watch_dir}{Colors.ENDC}")
        print(f"Debug Mode: {'FULL (NonSemantic -gVS)' if self.debug_mode else 'Off'}")
        print(f"Required Extension: VK_KHR_shader_non_semantic_info")
        print("-" * 60)

    def _find_compiler(self):
        # -gVS 플래그를 지원하는 glslangValidator를 우선적으로 찾습니다.
        for compiler in ['glslangValidator', 'glslc']:
            try:
                cmd = ['where', compiler] if os.name == 'nt' else ['which', compiler]
                result = subprocess.run(cmd, capture_output=True, text=True, check=True, timeout=5)
                return compiler
            except:
                continue
        return 'glslangValidator'

    def on_modified(self, event):
        if not event.is_directory and Path(event.src_path).suffix in self.shader_extensions:
            self.compile_shader(Path(event.src_path))

    def get_output_path(self, shader_path):
        relative_path = shader_path.relative_to(self.watch_dir)
        return self.output_dir / f"{relative_path}.spv"

    def compile_shader(self, shader_path):
        output_path = self.get_output_path(shader_path)
        timestamp = datetime.now().strftime("%H:%M:%S")

        try:
            output_path.parent.mkdir(parents=True, exist_ok=True)
            
            # glslangValidator 기반 명령어 구성 (매뉴얼 기준)
            if self.compiler_path == 'glslangValidator':
                cmd = [self.compiler_path, '-V', str(shader_path), '-o', str(output_path), '--target-env', 'vulkan1.3']
                if self.debug_mode:
                    # 매뉴얼 핵심: Nsight Flame Graph 및 심볼 로드를 위해 -gVS 사용
                    cmd.append('-gVS') 
                if self.optimize_mode:
                    # 최적화가 필요할 경우 (보통 디버그 시에는 제외)
                    pass 
            else:
                # glslc를 사용할 경우 (최후의 수단)
                cmd = [self.compiler_path, str(shader_path), '-o', str(output_path), '--target-env=vulkan1.3']
                if self.debug_mode:
                    cmd.append('-g')
                    cmd.append('-O0')

            result = subprocess.run(cmd, capture_output=True, text=True, timeout=30)
            
            if result.returncode == 0:
                print(f"[{timestamp}] {Colors.OKGREEN}✓ Compiled:{Colors.ENDC} {shader_path.name}")
            else:
                print(f"[{timestamp}] {Colors.FAIL}✗ Error:{Colors.ENDC} {shader_path.name}")
                print(f"{result.stderr or result.stdout}")
                        
        except Exception as e:
            print(f"✗ Unexpected error: {e}")

    def compile_all_shaders(self):
        all_files = [p for ext in self.shader_extensions for p in self.watch_dir.rglob(f"*{ext}")]
        for f in all_files: self.compile_shader(f)

def main():
    parser = argparse.ArgumentParser()
    parser.add_argument("watch_dir", nargs='?', default=None)
    parser.add_argument("-o", "--output", dest="output_dir", default=None)
    parser.add_argument("-g", "--debug", action="store_true", help="Enable -gVS for Nsight Debugging")
    args = parser.parse_args()

    watch_directory = Path(args.watch_dir) if args.watch_dir else Path.cwd()
    event_handler = ShaderCompilerHandler(watch_directory, args.output_dir, args.debug)
    observer = Observer()
    observer.schedule(event_handler, str(watch_directory), recursive=True)
    
    event_handler.compile_all_shaders()
    observer.start()
    
    print(f"{Colors.BOLD}Shader Watcher Active. Press Ctrl+C to exit.{Colors.ENDC}")
    try:
        while True: time.sleep(1)
    except KeyboardInterrupt:
        observer.stop()
    observer.join()

if __name__ == "__main__":
    main()