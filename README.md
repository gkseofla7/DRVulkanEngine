# Vulkan Renderer: Generalization & Optimization

본 프로젝트는 하드코딩된 구조에서 벗어나, **셰이더 리플렉션(Shader Reflection)**과 **렌더 그래프(Render Graph)**를 기반으로 설계된 현대적이고 유연한 Vulkan 렌더링 엔진입니다. 기존 D3D12 엔진 개발 경험을 바탕으로 리소스 관리 자동화와 파이프라인 최적화를 달성했습니다.

---

## 🛠 주요 기술 및 최적화 (Key Features)

### 1. Bindless 아키텍처 및 디스크립터 인덱싱 (Descriptor Indexing)
리소스 바인딩 주체를 CPU에서 GPU로 이전하여 **CPU 오버헤드와 GPU 파이프라인 중단(Stall)**을 획기적으로 개선했습니다.
* **통합 디스크립터 셋:** 모든 텍스처 리소스를 하나의 거대한 배열로 단일 디스크립터 셋에 미리 바인딩하여 잦은 디스크립터 교체를 방지합니다.
* **Push Constants 활용:** 무거운 디스크립터 셋 교체 대신, 가벼운 Push Constants를 통해 리소스 인덱스 정보만을 GPU에 전달합니다.
* **동적 인덱싱:** 셰이더는 전달받은 인덱스를 사용하여 텍스처 배열에서 필요한 리소스를 직접 선택해 샘플링합니다.

### 2. Shader Reflection 기반 리소스 관리 자동화
Vulkan API의 번거로운 리소스 레이아웃 정의 과정을 **셰이더 리플렉션**을 통해 자동화했습니다.
* **설계도 기반 자동화:** 셰이더(.glsl) 코드에 명시된 레이아웃 정보를 직접 읽어와 C++의 파이프라인 및 디스크립터 셋 생성 로직에 자동으로 반영합니다.
* **유지보수성 향상:** 셰이더 수정 시 C++ 코드를 수동으로 변경할 필요가 없어 휴먼 에러를 방지하고 개발 생산성을 높였습니다.

### 3. 리소스 상태 및 배리어 자동화
이미지 레이아웃 전환(Transition)과 메모리 의존성 해결을 위한 배리어 설정을 시스템화했습니다.
* **최적화된 상태 관리:** `VkImageLayout`, `VkAccessFlags2`, `VkPipelineStageFlags2`를 활용하여 렌더 타겟, 셰이더 읽기 등 목적에 맞는 최적화 상태를 자동으로 관리합니다.
* **데이터 경합 방지:** 파이프라인 단계별 접근 유형을 명시적으로 지정하여 데이터 레이스(Data Race)를 차단합니다.

### 4. 데이터 주도형 Render Graph 시스템
렌더링 패스의 순서와 의존성을 구조화하여 **워크플로우의 범용성**을 확보했습니다.
* **유연한 패스 구성:** JSON 기반 설계를 통해 엔진 코드 수정 없이 렌더링 노드(SSAO, ShadowMap, Opaque 등)의 순서를 제어할 수 있습니다.
* **Dynamic Rendering:** 최신 Vulkan 기능인 `vkCmdBeginRendering`을 적용하여 렌더 패스 관리의 복잡도를 낮추고 MSAA Resolve 로직을 통합했습니다.

---

##  자세한 설명(Blog)
* **Blog Link:** [Tech Blog](https://gkseofla7.tistory.com/11)

