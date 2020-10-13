- [DB_bptree](#db_bptree)
  - [과제 설명](#과제-설명)
  - [구현 내용](#구현-내용)
    - [개발 환경](#개발-환경)
    - [구현 및 미구현](#구현-및-미구현)
    - [구현에 대한 간략한 설명](#구현에-대한-간략한-설명)
    - [컴파일 및 실행 방법](#컴파일-및-실행-방법)
    - [수행 경험](#수행-경험)
  - [연락처](#연락처)

# DB_bptree
DATABASE Project #2, implementation to binary plus tree

## 과제 설명

<img width="784" alt="스크린샷 2020-07-03 오후 10 35 27" src="https://user-images.githubusercontent.com/44011462/86474473-d9271400-bd7d-11ea-9196-be96a0614c69.png">
<img width="859" alt="스크린샷 2020-07-03 오후 10 35 42" src="https://user-images.githubusercontent.com/44011462/86474481-da584100-bd7d-11ea-8410-e882e99b8c0a.png">
<img width="796" alt="스크린샷 2020-07-03 오후 10 35 48" src="https://user-images.githubusercontent.com/44011462/86474487-dc220480-bd7d-11ea-864e-4910be48e656.png">
<img width="796" alt="스크린샷 2020-07-03 오후 10 35 56" src="https://user-images.githubusercontent.com/44011462/86474489-dd533180-bd7d-11ea-8532-dab40c3d9d45.png">
<img width="796" alt="스크린샷 2020-07-03 오후 10 36 02" src="https://user-images.githubusercontent.com/44011462/86474492-ddebc800-bd7d-11ea-85ae-b62689802904.png">
<img width="796" alt="스크린샷 2020-07-03 오후 10 36 09" src="https://user-images.githubusercontent.com/44011462/86474493-de845e80-bd7d-11ea-947c-a8eace1148fd.png">
<img width="796" alt="스크린샷 2020-07-03 오후 10 36 16" src="https://user-images.githubusercontent.com/44011462/86474496-df1cf500-bd7d-11ea-8907-0bb882474655.png">
<img width="871" alt="스크린샷 2020-07-03 오후 10 36 26" src="https://user-images.githubusercontent.com/44011462/86474497-dfb58b80-bd7d-11ea-8b5c-bed2e9c390a9.png">
<img width="871" alt="스크린샷 2020-07-03 오후 10 36 46" src="https://user-images.githubusercontent.com/44011462/86474499-e04e2200-bd7d-11ea-8b52-321258779f18.png">
<img width="871" alt="스크린샷 2020-07-03 오후 10 37 02" src="https://user-images.githubusercontent.com/44011462/86474500-e04e2200-bd7d-11ea-8f8f-1c32dfa5a249.png">
<img width="871" alt="스크린샷 2020-07-03 오후 10 37 09" src="https://user-images.githubusercontent.com/44011462/86474502-e0e6b880-bd7d-11ea-96de-4e4c041c978b.png">
<img width="871" alt="스크린샷 2020-07-03 오후 10 37 23" src="https://user-images.githubusercontent.com/44011462/86474503-e0e6b880-bd7d-11ea-8430-a1572175c3d2.png">
<img width="871" alt="스크린샷 2020-07-03 오후 10 37 31" src="https://user-images.githubusercontent.com/44011462/86474504-e17f4f00-bd7d-11ea-8bad-e6fe50f58015.png">

## 구현 내용

### 개발 환경

개발언어: c++
개발환경: macOS catalina 10.15.5, clion 2019.2.9

### 구현 및 미구현
- bptree 데이터는 bptree.bin파일에 저장되도록 하였습니다.
- 모든 bptree의 데이터가 main memory에 한꺼번에 올리지 않으며, 필요에 의해 사용합니다.
- 하나의 노드의 크기를 블럭의 사이즈에 맞게 구현했습니다.
- 모든 블럭은 고유의 BID로 구분되며, 4bytes의 크기로 나타냈습니다.
- BID는 1부터 시작하며, 0은 NULL block를 의미합니다.
- bptree파일의 물리적 주소는 12 + ((BID - 1) * blockSize)로 구할 수 있습니다.
- node가 가지는 entry의 개수는 (blockSize - 4) / 8로 구할 수 있습니다.

그밖에 과제의 구현 요구사항으로 주어진 모든 내용을 구현하였습니다. 


### 구현에 대한 간략한 설명
- main함수 부분에는 간략한 함수선언만 들어있도록 작성했습니다. 
  - terminal로 bptree.exe를 실행하면 파일의 위치로 이동해 main의 arguments로 c|i|s|r|p등을 입력 받고 수행하도록 만들었다. 
  - 정해진 argument의 수가 입력되지 않으면 실행 사용법을 출력하며 bptree.exe를 종료합니다.
  -  Bptree를 최초 실행하면 Binary file을 생성을 합니다. 
     -  처음에는 Node를 생성하고 Internal Node와 Leaf Node는 Node 클래스를 상속 받아 수행하도록 했습니다.
- 기본 Node에는 Binary file에서 읽어온 글자를 담거나 이진 파일에 쓸 글자를 담을 공간 memory_chunk, block_size(용량), bid, cap(수용량), size(현재 원소 개수)를 담고 있습니다. 
  - non-leaf node는 nextlevelBid를 추가하였고 leaf node는 nextBid를 추가했습니다. 이 두 가지 node 모두는 node를 상속 받습니다. 
  - leaf node에서 add는 반환형이 boolean 형태이며 block이 꽉차 있는지, 혹은 같은 key값이 있는지 확인합니다. 
    - 같은 값이 있다면 false를 반환해 split_add를 사용하고 
    - 같은 값이 없다면, 그 block에 key값과 value 값을 추가합니다. 
  - Split_add는 배열 하나를 만들어, 2개의 block 크기 만큼 수를 집어 넣고, key값과 value 값을 집어 넣은 후, 중간값과 끝 값을 찾아 2개로 다시 분리 시킵니다. 
  - Find 함수는 그 key 값이 존재할 block의 모든 값들을 찾아 반환합니다. 
- Non-leaf node, internal node는 다음 block을 가리키기 위한 함수 get_next_bid()가 있습니다. 
  - get_next_bid()는 Leaf node 시작점의 주소값을 저장합니다. 
  - Add와 split은 leaf와 유사하지만 split에서는 조건이 추가됩니다. 
  Leaf node에 버킷이 꽉차서 2개로 쪼개고 가운데 키를 부모 노드에 추가하려 했을 때, 
    - 그 부모 non-leaf node 또한 꽉찬 경우 이 non-leaf node의 부모노드 또한 쪼개서 중간키를 자신의 부모노드에 추가합니다.
  
- 마지막은 btree입니다. 
  - header의 정보를 입력하는 함수 update_header가 있습니다. 
  - 파일에서 block을 읽어오는 함수 load_block이 존재합니다. 
  - Demo에 있는 것처럼 수식을 이용해, block을 읽습니다. 
  - search함수는 key, bid, depth값을 받아 depth가 현재 btree의 depth랑 같다면 leaf_node(bid)를 반환합니다. 
  - Root bid, depth=0부터 시작하므로 btree의 depth가 같을 때까지 타고 들어가며 depth++ 해주고, bid는 depth가 같지 않다면 non-leaf node이므로 next bid를 가리키게 합니다.
  - Insert도 유사하게 진행됩니다. Search와 같이 재귀를 돌리며, btree의 depth와 현재 depth가 같다면 block이 꽉 차있다면 split, 아니면 바로 삽입합니다. 
  - Print 함수는 root부터 시작해 root에 있는 모든 원소를 출력 한 후에 다시 처음부터 파일을 읽고 root의 원소를 하나씩 체크합니다.
    -  깊이가 1이냐 그 이상이냐 2가지로 나눠, 깊이가 1이면 leaf node의 nextbid들을 찾아 쭉 나타내고
    -  아니라면 non-leaf node의 nextLevelBid를 찾아 원소의 수만큼 출력합니다. 
  - range_search는 첫 key값을 받아 그 위치를 찾고, key2 값보다 작을 때까지 key와 value 값의 페어를 출력합니다. Key2가 더 커지면 bool을 이용해 반복문을 탈출했습니다.

### 컴파일 및 실행 방법
1. terminal을 이용하여 파일 위치로 이동합니다.
2. 실행파일.exe c|i|s|r|p 에 맞춰 이진파일 이름 혹은 입력할 text 파일을 쓴다.
3. 원하는 결과를 얻습니다.

### 수행 경험
Binary Tree, Trie등의 Tree구조를 연습했지만 Bptree의 제작은 상당히 까다로웠습니다. Block이 꽉 차면 새로운 Block을 만들어 할당하는 부분이 어려웠습니다. 특히 파일을 입력받고 출력하는 부분은 Bptree를 구현하는 것만큼 많은 시간을 할애하여 공부했습니다. Block을 확장한다면 이 전에 있던 block의 원소를 가지고 오는 것이 아니라 그 뒤에 있는 원소를 가지고 오는 것으로, 그 부분 수정하는 것도 상당히 힘들었습니다. 논리적인 오류가 어떤 부분에서 발생했는지 디버깅하기에도 버거워서, 여러차례 코드를 처음부터 다시 작성하는 시행착오를 겪었습니다. 많은 시간을 투자한 만큼 Bptree에 대해 보다 깊이 있게 이해할 수 있었습니다.

## 연락처
CSE 12164720 정경하
justin.jeong5@gmail.com
010 7273 4775
