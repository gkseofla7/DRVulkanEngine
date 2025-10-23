import fbx
import os
import sys # sys 모듈 추가

def extract_textures_from_fbx(fbx_filepath, output_dir):
    # (함수 내부는 기존 코드와 동일)
    # ... (생략) ...
    manager = fbx.FbxManager.Create()
    scene = fbx.FbxScene.Create(manager, "MyScene")
    importer = fbx.FbxImporter.Create(manager, "")
    if not importer.Initialize(fbx_filepath):
        print(f"Error: {importer.GetStatus().GetErrorString()}")
        return
    importer.Import(scene)
    importer.Destroy()
    if not os.path.exists(output_dir):
        os.makedirs(output_dir)
    print(f"'{os.path.basename(fbx_filepath)}' 파일에서 텍스처를 스캔합니다...")
    for i in range(scene.GetTextureCount()):
        texture = scene.GetTexture(i)
        if isinstance(texture, fbx.FbxVideo) and texture.IsEmbedded():
            filename = texture.GetRelativeFileName()
            if not filename:
                filename = texture.GetFileName()
            content = texture.GetMedia().GetContent()
            output_path = os.path.join(output_dir, os.path.basename(filename))
            with open(output_path, "wb") as f:
                f.write(content)
            print(f"  -> 추출 완료: {output_path} ({len(content) / 1024:.2f} KB)")
    manager.Destroy()

# --- 실행 부분 ---
if __name__ == "__main__":
    # 명령줄에서 인자를 받도록 수정
    if len(sys.argv) < 2:
        print("사용법: py -3.10 your_script_name.py <FBX_파일_경로> [출력_폴더_경로]")
        sys.exit() # 인자가 없으면 프로그램 종료

    # 첫 번째 인자를 fbx 파일 경로로 사용
    fbx_file = sys.argv[1]
    
    # 두 번째 인자가 있으면 출력 폴더로 사용, 없으면 기본값 사용
    output_folder = sys.argv[2] if len(sys.argv) > 2 else "./extracted_textures"

    extract_textures_from_fbx(fbx_file, output_folder)