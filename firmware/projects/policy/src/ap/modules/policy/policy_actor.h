#ifndef POLICY_ACTOR_H_
#define POLICY_ACTOR_H_

// 두 단계 정책의 forward 함수. 생성된 actor.h 는 같은 이름의 static 배열을 쓰므로
// 각각 다른 .c 파일에 넣고 이 함수로만 부른다.

#define POLICY_OBS_SIZE     4
#define POLICY_ACTION_SIZE  2

void actorPhase1Forward(const float *obs, float *action);
void actorPhase2Forward(const float *obs, float *action);

#endif
