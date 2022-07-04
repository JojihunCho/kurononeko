- global variable 추가
Pre_drag = False (H_DRAG 초기 실행 시점 체크를 위함)
Cow_point = [] (경로를 만들기 위한 6개의 위치를 저장)
Cow_init = [] (6개의 위치 중에서 애니메이션이 끝나고 복원할 1번째 위치를 복사)
Set_control = False (처음 Cow를 클릭할 때 그 위치가 바로 저장되는 것을 방지)
Start_animation = False (경로 입력이 끝나고 애니메이션 시작을 알림)
animStartTime = 0 (애니메이션 시작 시간을 저장)
timeCheck = True (애니메이션 시작 시간을 저장 해야 하는지 알림)
Animation_running = False (애니메이션이 작동 중인지 알림)

- onMouseDrag(window, x, y) 변경
if isDrag: (해당 조건문 내부의 조건문을 변경)
if isDrag==V_DRAG: (마우스의 왼쪽 버튼을 누른 상태로 드래그를 했을 때 수직으로 cow가 이동하도록 기능 추가)
p=Plane(np.array((0,0,1)), getTranslation(cow2wld)) (vertical dragging과 비슷한 방법으로 Plane을 설정해서 이동하는 것을 구현)
horizontal dragging (기존 코드에 Plane을 만드는 방법을 변경하여 vertical dragging에 의해 바뀐 위치를 기준으로 움직이도록 변경)
if Pre_drag: (vertical dragging 직후에 horizontal dragging가 실행 될 경우 horizontal로 움직일 새로운 Plane을 설정)

- onMouseButton(window,button, state, mods) 변경
if button == glfw.MOUSE_BUTTON_LEFT: (왼쪽 마우스 버튼을 눌렀을 때 드래그 모드를 변경하도록 기능 추가)
elif state == GLFW_UP and isDrag!=0: (왼쪽 마우스 버튼을 땠을 때, 처음 cow를 클릭한 상황인지, 아무것도 아닌 상황인지, cow의 point를 지정하는 상황인지 확인하여 다르게 작동)
if Set_control: (만약 cow의 control point를 설정하는 상황이면 전역 변수 Cow_point에 해당 값을 저장, 값이 6개가 되었다면 amination을 작동)

- display() 기능 추가
if Start_animation: (cow가 point를 따라 움직여야 한다면 작동, 움직일 필요가 없다면 저장된 point에 cow 그리기)
if timeCheck: (Animation을 진행할 때 초기값을 설정 - Animation마다 1번만 실행)
if Animation_running: (Animation이 작동하는 동안 반복해서 진행. Animation 작동 시간을 계산하고, 그에 따른 움직인 cow를 그리고 함수를 종료. Animation이 끝난 상황이면 사용한 값들을 초기화)
running_time = glfw.get_time() - Start_animation (Animation 작동 시간을 계산. 1cycle에 6초, 총 18초 동안 cow가 정해진 point를 따라 움직인다. 속도를 조절하려면 해당 값에 원하는 상수를 곱함)

- Catmull_Rom_spline(running_time): 함수 추가
전역 변수인 Cow_point와 매개 변수인 running_time를 이용하여 Catmull_Rom_spline을 진행하는 함수.
기본적으로 해당 시점에 cow가 지나는 point와 0.0001초 후 cow가 지나는 point를 반환. 
1초에 1개의 point를 지나도록 구현되어 있으면 18초(3 cycle)가 지나면 false를 반환.
해당 시점에 cow가 지나는 point는 display()에서 setTranslation(m, v)에 바로 이용됨.
0.0001초 후 cow가 지나는 point는 해당 시점에 cow가 지나는 point와 함께 makeRotation(p0, p1)에서 회전 행렬을 만드는데 이용.

- makeRotation(p0, p1): 함수 추가
p0에서 p1를 향해 바라보는 회전 행렬을 반환하는 함수
math.atan2을 이용하여 pitch와 roll에 대한 라디안 값을 계산, pitch와 roll을 표현하는 행렬을 만들고,
해당 행렬을 정규화 시켜서 반환
pitch에 대한 라디안 값을 계산을 계산할 때, x 축에 대한 z값을 기준으로 계산 
roll에 대한 라디안 값을 계산을 계산할 때, xz 평면에 대한 y값을 기준으로 계산

- getRotation(m): 함수 추가
- setRotation(m, v): 함수 추가
getTranslation(m)과 setTranslation(m,v)처럼 행렬 m의 일부를 반환하거나 v로 바꿔주는 함수
