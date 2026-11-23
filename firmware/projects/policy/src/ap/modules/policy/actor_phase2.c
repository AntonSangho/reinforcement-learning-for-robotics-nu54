/* actor_phase2.c — 2단계 정책 (best_model). actor.h 의 static 가중치를 이 파일 안에 가둔다 */

#include "actor_phase2.h"
#include "policy_actor.h"


void actorPhase2Forward(const float *obs, float *action)
{
  actor_forward(obs, action);
}
