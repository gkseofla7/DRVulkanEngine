import bpy
import sys
import os

def clean_and_export_fbx(input_path, output_path):
    # 1. 씬 초기화 (빈 상태로 시작)
    bpy.ops.wm.read_factory_settings(use_empty=True)

    # 2. FBX 임포트
    print(f"--- 파일 읽기 시작: {input_path} ---")
    bpy.ops.import_scene.fbx(filepath=input_path)

    # 3. 이상한 노드($AssimpFbx$)들 체크 및 처리
    # 사실 Blender로 임포트하는 과정에서 많은 Dummy 노드가 자동으로 정리됩니다.
    # 추가적으로 모든 오브젝트를 순회하며 이름을 확인합니다.
    for obj in bpy.data.objects:
        if "$AssimpFbx$" in obj.name:
            print(f"정리 대상 노드 발견: {obj.name}")
            # 이 노드들은 내보낼 때 'add_leaf_bones=False'와 
            # 'simplify_factor' 옵션에 의해 대부분 제거/통합됩니다.

    # 4. 깨끗한 상태로 엑스포트 (가장 중요한 부분)
    print(f"--- 클리닝 및 저장 시작: {output_path} ---")
    
    bpy.ops.export_scene.fbx(
        filepath=output_path,
        use_selection=False,
        object_types={'ARMATURE', 'MESH'}, # 카메라, 조명 제외
        
        # ★ 핵심 설정: 가짜 뼈(Leaf Bones) 생성 안 함 ★
        add_leaf_bones=False, 
        
        # 좌표계 설정 (Vulkan/Unreal 표준: Y-Up, -Z Forward)
        axis_forward='-Z',
        axis_up='Y',
        
        # 텍스처 추출을 위해 데이터 포함
        path_mode='COPY',
        embed_textures=True,
        
        # 애니메이션 베이크 및 단순화 (불필요한 키프레임 제거)
        bake_anim=True,
        bake_anim_simplify_factor=1.0,
        bake_anim_use_all_actions=True
    )

if __name__ == "__main__":
    # 인자 처리: blender --bg --python clean_fbx.py -- [입력] [출력]
    try:
        argv = sys.argv
        idx = argv.index("--")
        input_fbx = argv[idx + 1]
        output_fbx = argv[idx + 2]
    except (ValueError, IndexError):
        print("에러: 입력 또는 출력 경로가 누락되었습니다.")
        print("사용법: blender --background --python clean_fbx.py -- <입력_FBX> <출력_FBX>")
        sys.exit(1)

    clean_and_export_fbx(input_fbx, output_fbx)
    print(f"--- 모든 작업 완료: {os.path.basename(output_fbx)} ---")