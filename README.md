# 컴퓨터 그래픽스

2022년 1학기에 수강한 그래픽스 과제입니다.

Assignment 1 ~ 7은 OpenGL을 이용한 실습을 했던 파일들입니다.

PA1 Ray tracer
---

해당 과제는 XML 파일을 읽고, Ray tracing을 통해 이미지 파일로 만드는 프로그램입니다.

camera point로부터 각 물체에 대한 ray를 구한 뒤 픽셀 좌표로 변환하여, 각 픽셀마다의 색상과 쉐이딩 결과를 저장하여 이미지 파일로 출력합니다.

PA2 Cow roller coaster
---

스켈레톤 코드를 제공받아, 소 Object를 사용자가 클릭한 위치를 롤러코스터 타듯 움직이게 하는 과제입니다.

사용자가 소 Object를 움직여 6번 클릭했을 때의 위치를 기억하고 . Catmull-rom spline를 이용하여 소 Object를 움직입니다. 

Yaw와 Pitch를 이용해 소 Ojbect를 다음 움직일 위치에 맞춰 회전시킵니다.

자세한 구현은 PA2에 [README](https://github.com/heegh000/HYU-Computer-Graphics/blob/master/PA2_2022_2018009234/README.pdf)에 작성되어 있습니다. 
