import os
import time
import subprocess
import sys
import argparse
from pathlib import Path
from datetime import datetime
from watchdog.observers import Observer
from watchdog.events import FileSystemEventHandler

# 콘솔 출력에 사용할 ANSI 컬러 코드
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
        self.glslc_path = self._find_glslc()

        print(f"{Colors.HEADER}--- Shader Watcher Initialized ---{Colors.ENDC}")
        print(f"Monitoring: {Colors.OKBLUE}{self.watch_dir}{Colors.ENDC}")
        print(f"Output to:  {Colors.OKBLUE}{self.output_dir}{Colors.ENDC}")
        print(f"Debug Mode (-g): {'On' if self.debug_mode else 'Off'}")
        print(f"Optimize Mode (-O): {'On' if self.optimize_mode else 'Off'}")
        print("-" * 50)

    def _find_glslc(self):
        try:
            cmd = ['where', 'glslc'] if os.name == 'nt' else ['which', 'glslc']
            result = subprocess.run(cmd, capture_output=True, text=True, check=True, timeout=5)
            return result.stdout.strip().split('\n')[0]
        except (subprocess.CalledProcessError, FileNotFoundError, subprocess.TimeoutExpired):
            return 'glslc' # PATH에서 찾지 못할 경우 기본값 사용

    def on_created(self, event):
        if not event.is_directory and Path(event.src_path).suffix in self.shader_extensions:
            self.compile_shader(Path(event.src_path))

    def on_modified(self, event):
        if not event.is_directory and Path(event.src_path).suffix in self.shader_extensions:
            self.compile_shader(Path(event.src_path))

    def on_deleted(self, event):
        if not event.is_directory and Path(event.src_path).suffix in self.shader_extensions:
            self.clean_spv(Path(event.src_path))

    def on_moved(self, event):
        if not event.is_directory:
            src_path = Path(event.src_path)
            dest_path = Path(event.dest_path)
            if src_path.suffix in self.shader_extensions:
                self.clean_spv(src_path)
            if dest_path.suffix in self.shader_extensions:
                self.compile_shader(dest_path)

    def get_output_path(self, shader_path):
        relative_path = shader_path.relative_to(self.watch_dir)
        return self.output_dir / f"{relative_path}.spv"

    def clean_spv(self, shader_path):
        output_path = self.get_output_path(shader_path)
        if output_path.exists():
            timestamp = datetime.now().strftime("%H:%M:%S")
            print(f"[{timestamp}] {Colors.WARNING}Cleaning: Deleting stale {output_path.name}{Colors.ENDC}")
            output_path.unlink()

    def compile_shader(self, shader_path):
        output_path = self.get_output_path(shader_path)
        timestamp = datetime.now().strftime("%H:%M:%S")

        try:
            if output_path.exists() and output_path.stat().st_mtime > shader_path.stat().st_mtime:
                return # 최신 버전이므로 컴파일 건너뛰기

            output_path.parent.mkdir(parents=True, exist_ok=True)
            
            cmd = [self.glslc_path, str(shader_path), '-o', str(output_path)]
            if self.debug_mode:
                cmd.append('-g') # RenderDoc 디버깅을 위한 디버그 심볼 포함
            if self.optimize_mode:
                cmd.append('-O') # 성능 최적화

            print(f"[{timestamp}] {Colors.BOLD}Compiling:{Colors.ENDC} {shader_path.name} -> {output_path.name}")
            
            result = subprocess.run(cmd, capture_output=True, text=True, timeout=30)
            
            if result.returncode == 0:
                print(f"[{timestamp}] {Colors.OKGREEN}✓ Success:{Colors.ENDC} {output_path.name}")
                if result.stdout.strip():
                    print(f"  Output: {result.stdout.strip()}")
            else:
                print(f"[{timestamp}] {Colors.FAIL}✗ Error compiling {shader_path.name}:{Colors.ENDC}")
                error_output = (result.stderr or '') + (result.stdout or '')
                if error_output.strip():
                    # 에러 메시지를 여러 줄로 깔끔하게 출력
                    for line in error_output.strip().split('\n'):
                        print(f"  {Colors.FAIL}| {line}{Colors.ENDC}")
                        
        except Exception as e:
            print(f"[{timestamp}] {Colors.FAIL}✗ Unexpected error: {e}{Colors.ENDC}")

    def compile_all_shaders(self):
        print(f"[{datetime.now().strftime('%H:%M:%S')}] Running initial compilation scan...")
        
        all_files = [p for ext in self.shader_extensions for p in self.watch_dir.rglob(f"*{ext}")]
        
        if not all_files:
            print("No shader files found.")
            return

        compiled_count = 0
        for shader_file in all_files:
            output_path = self.get_output_path(shader_file)
            if not output_path.exists() or output_path.stat().st_mtime <= shader_file.stat().st_mtime:
                self.compile_shader(shader_file)
                compiled_count += 1
        
        print(f"[{datetime.now().strftime('%H:%M:%S')}] Initial scan complete. "
              f"Compiled {compiled_count}/{len(all_files)} shader files.")
        print("-" * 50)

def main():
    parser = argparse.ArgumentParser(description="Vulkan Shader Auto-Compiler using glslc.")
    parser.add_argument("watch_dir", nargs='?', default=None,
                        help="Directory to watch for shader files. Defaults to './shaders' or current directory.")
    parser.add_argument("-o", "--output", dest="output_dir", default=None,
                        help="Output directory for compiled SPV files. Defaults to the watch directory.")
    parser.add_argument("-g", "--debug", action="store_true",
                        help="Include debug information in SPV files for RenderDoc.")
    parser.add_argument("-O", "--optimize", action="store_true",
                        help="Optimize SPV files for performance.")
    args = parser.parse_args()

    # 감시할 디렉터리 결정 로직
    if args.watch_dir:
        watch_directory = Path(args.watch_dir)
    else:
        script_dir = Path(__file__).parent
        watch_directory = script_dir / "shaders"
        if not watch_directory.is_dir():
            watch_directory = Path.cwd()

    if not watch_directory.is_dir():
        print(f"{Colors.FAIL}Error: Directory '{watch_directory}' does not exist.{Colors.ENDC}")
        sys.exit(1)

    # glslc 존재 여부 확인
    try:
        result = subprocess.run(['glslc', '--version'], capture_output=True, text=True, check=True, timeout=10)
        print(f"Found glslc: {result.stdout.strip().splitlines()[0]}")
    except Exception:
        print(f"{Colors.FAIL}Error: glslc not found. Please ensure Vulkan SDK is installed and in your PATH.{Colors.ENDC}")
        sys.exit(1)

    # 이벤트 핸들러 및 옵저버 생성/시작
    event_handler = ShaderCompilerHandler(watch_directory, args.output_dir, args.debug, args.optimize)
    observer = Observer()
    observer.schedule(event_handler, str(watch_directory), recursive=True)
    
    event_handler.compile_all_shaders()
    
    observer.start()
    print(f"{Colors.BOLD}Watching for shader file changes... (Press Ctrl+C to stop){Colors.ENDC}")
    
    try:
        while True:
            time.sleep(1)
    except KeyboardInterrupt:
        print(f"\n{Colors.WARNING}Stopping shader watcher...{Colors.ENDC}")
        observer.stop()
    
    observer.join()
    print("Shader watcher stopped.")

if __name__ == "__main__":
    main()