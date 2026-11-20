#pragma once
#include "Game.h"
#include "Util/Obj.h"

// 렌더링에 필요한 GPU 리소스와 카메라. 게임 로직은 바꾸지 않는다.

// 화면 UI 사각형 (창 좌표: 왼쪽 위가 원점, y는 아래로)
struct Rect
{
	float x = 0.f, y = 0.f, w = 0.f, h = 0.f;
};

constexpr float PackAnimTime = 0.7f;  // 카드팩에서 나온 카드가 손패에 들어가기까지(초)

// 화면 전환: 카드를 놓은 곳에서 원이 퍼져 화면을 덮고(In), 잠깐 멈췄다가(Hold), 걷힌다(Out)
constexpr float TransitionIn = 0.45f, TransitionHold = 0.12f, TransitionOut = 0.5f;

// 파티클 한 종류의 수치 (Data/effects.csv 한 줄)
struct EffectData
{
	bool ring = false;            // true: 바닥을 따라 둥글게 퍼짐, false: 사방으로 터짐
	int count = 0;
	float speedMin = 1.f, speedMax = 1.f;
	float lifeMin = 0.5f, lifeMax = 0.5f;
	float sizeStart = 0.1f, sizeEnd = 0.f;
	glm::vec4 colorStart = glm::vec4(1.f), colorEnd = glm::vec4(1.f);
	float gravity = 0.f;
};

struct Particle
{
	glm::vec3 position, velocity;
	float age = 0.f, life = 1.f;
	int effect = 0;               // EventType 번호 = effects 번호
};

struct Renderer
{
	GLuint program = 0;
	Mesh axes, cube, quad, disc, circle, roundTile;
	std::vector<Model> unitModels;  // units.csv 순서
	std::vector<Model> mobModels;   // mobs.csv 순서
	Camera camera;
	bool isOrtho = false;
	bool toon = true;              // 툰 셰이딩 (T 키로 퐁과 바꿔 보기)
	bool shadows = true;           // 그림자 맵 (G 키로 끄고 켜기)
	int speed = 1;                 // 게임 배속 1~3 (배속 버튼, F 키)
	GLuint shadowFbo = 0, shadowMap = 0;  // 빛에서 본 깊이를 담는 프레임버퍼와 깊이 텍스처
	int hoverI = -1, hoverJ = -1;  // 마우스가 가리키는 칸 (-1이면 없음)

	// 판 텍스처
	GLuint grassTexture = 0, stoneTexture = 0, woodTexture = 0, tableTexture = 0;

	// UI 텍스처
	std::vector<GLuint> unitCards;  // units.csv 순서
	std::vector<GLuint> spellCards; // spells.csv 순서
	std::vector<GLuint> relicIcons; // relics.csv 순서
	GLuint cardBack = 0, promoteCard = 0, goldPack = 0;
	GLuint startCard = 0, exitCard = 0, retryCard = 0, titleCard = 0;
	GLuint digits = 0, chipIcon = 0, booster = 0, endTurn = 0, logo = 0, victory = 0, defeat = 0;

	// 파티클: EventType 순서(Summon, Hit, Death, Meteor)로 수치를 둔다
	std::vector<EffectData> effects;
	std::vector<Particle> particles;
	GLuint dotTexture = 0;  // 가운데가 진하고 가장자리가 투명한 점

	// 카드팩 개봉 연출: 팩에서 뒤집히며 손패 자리로 날아가는 카드 (PackAnimTime초)
	struct PackAnim
	{
		int handIndex = -1;  // 손패 카드면 손패 번호, 유물이면 -1
		int relic = -1;      // 유물이면 relics.csv 행 번호
		float age = 0.f;
		float delay = 0.f;   // 한 팩에서 여러 장이 나오면 차례로 나오게
		int source = 0;      // 나온 곳: 0 부스터, 1 골드 부스터, 2 베팅 보상
		bool merged = false; // 같은 카드 3장을 합성한 카드 (빛나는 연출)
	};
	std::vector<PackAnim> packAnims;
	int packHandSize = 0;  // 마지막으로 팩을 산 직후의 손패 수
	float jackpotTimer = -1.f;  // 대박 연출이 시작된 뒤 지난 시간 (음수면 없음)
	int jackpotSource = 0;      // 대박이 터진 팩 (0 부스터, 1 골드)
	GLuint betOn = 0, betOff = 0;

	// 화면 전환 카드(Start/Exit/Retry/Title)를 놓았을 때. 화면이 다 덮이면 main이 카드를 실제로 쓴다
	struct Transition
	{
		bool active = false;
		bool applied = false;  // 카드를 이미 썼는지
		float time = 0.f;
		glm::vec2 origin = glm::vec2(0.f);  // 원이 퍼지기 시작하는 창 좌표
		int handIndex = -1, i = -1, j = -1;
	};
	Transition transition;

	// UI 입력 상태 (main이 채운다)
	glm::vec2 mouse = glm::vec2(0.f);  // 창 좌표
	int dragCard = -1;                 // 끌고 있는 손패 카드 번호 (-1이면 없음)
};

void RendererInit(Renderer& renderer);
glm::mat4 ViewMatrix(const Camera& camera);
glm::mat4 ProjectionMatrix(const Renderer& renderer, float aspect);
void RendererUpdate(Renderer& renderer, const Game& game, float dt);  // 게임 이벤트로 파티클을 만들고 움직인다
void RenderGame(const Renderer& renderer, const Game& game, int width, int height);
glm::mat4 LightSpaceMatrix();  // 월드 → 빛의 클립 좌표 (직교 투영 × 빛 시점)
void RenderBoard(const Renderer& renderer, const Game& game);
void RenderUnits(const Renderer& renderer, const Game& game);
void RenderMobs(const Renderer& renderer, const Game& game);
void RenderPreview(const Renderer& renderer, const Game& game);
void RenderParticles(const Renderer& renderer);  // 끌고 있는 유닛 카드의 반투명 말과 사거리 원

// 화면 UI. 그리기와 클릭 판정이 같은 위치를 쓰도록 배치 함수를 둔다 (width, height = 창 크기).
bool Contains(const Rect& r, glm::vec2 p);
Rect HandCardRect(int index, int count, int width, int height);
int HandCardAt(const Game& game, glm::vec2 p, int width, int height);  // 그 위치의 손패 카드 번호, 없으면 -1
Rect PackRect(int width, int height);
Rect GoldPackRect(int width, int height);  // 골드 부스터 (일반 카드팩 위)
Rect EndTurnRect(int width, int height);
Rect SpeedRect(int width, int height);  // 배속 버튼 (턴 종료 버튼 위)
Rect BetRect(int width, int height);    // 최대 베팅 스위치 (배속 버튼 위)
void RenderUi(const Renderer& renderer, const Game& game, int width, int height);
void StartTransition(Renderer& renderer, glm::vec2 origin, int handIndex, int i, int j);
bool TransitionCovered(const Renderer& renderer);  // 화면이 다 덮였고 아직 카드를 안 썼으면 true
