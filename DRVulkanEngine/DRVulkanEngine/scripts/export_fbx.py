import bpy
import sys
import os

def export_scene_to_fbx(output_path):
    """
    현재 Blender 씬을 지정된 설정으로 FBX 파일로 익스포트합니다.
    """
    
    # FBX 익스포트 실행
    # bpy.ops.export_scene.fbx() 함수의 모든 옵션은 Blender의 FBX 익스포트 메뉴와 동일합니다.
    bpy.ops.export_scene.fbx(
        filepath=output_path,
        
        # ★★★★★ 가장 중요한 설정: Leaf Bone 추가 비활성화 ★★★★★
        # 이 옵션을 False로 설정하면 '_$AssimpFbx$_'나 '_End' 같은 불필요한 노드가 생성되지 않습니다.
        add_leaf_bones=False,
        
        # --- 아래는 게임 개발에 유용한 일반적인 추천 설정입니다 ---
        
        # 좌표계 설정 (대부분의 게임 엔진은 Y-Up, Z-Forward를 사용합니다)
        axis_forward='-Z',
        axis_up='Y',
        
        # 메시와 아마추어(뼈대)만 익스포트 (카메라, 조명 등은 제외)
        object_types={'ARMATURE', 'MESH'},
        
        # 전체 씬이 아닌, 선택된 오브젝트만 익스포트할 경우 True로 변경
        use_selection=False,
        
        # 애니메이션 베이크(Bake) 설정
        bake_anim=True,                 # 애니메이션 데이터를 프레임 단위로 '굽습니다'.
        bake_anim_use_nla_strips=False, # NLA 트랙은 굽지 않음
        bake_anim_use_all_actions=True, # 모든 액션(애니메이션 클립)을 포함
        bake_anim_force_startend_keying=False,
    )


if __name__ == "__main__":
    # --- 명령줄 인수(Argument) 처리 ---
    # Blender를 --background 모드로 실행하면 -- 뒤에 오는 인수를 파이썬 스크립트에서 받을 수 있습니다.
    try:
        # '--' 구분자 뒤의 첫 번째 인수를 출력 경로로 사용합니다.
        argv = sys.argv
        output_filepath = argv[argv.index("--") + 1]
    except (ValueError, IndexError):
        print("에러: 출력 파일 경로가 지정되지 않았습니다.")
        print("사용법: blender --background <.blend 파일> --python <스크립트.py> -- <출력.fbx 파일>")
        sys.exit(1)

    print("-" * 50)
    print(f"자동 FBX 익스포트를 시작합니다...")
    print(f"입력 파일 (현재 씬): {bpy.data.filepath}")
    print(f"출력 파일: {output_filepath}")

    # 익스포트 함수 호출
    export_scene_to_fbx(output_filepath)

    print(f"성공적으로 '{os.path.basename(output_filepath)}' 파일을 생성했습니다.")
    print("-" * 50)