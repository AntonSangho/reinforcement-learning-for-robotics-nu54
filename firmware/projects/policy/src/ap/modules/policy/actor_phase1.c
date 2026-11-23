/* actor_phase1.c — 1단계 정책 (best_model). actor.h 의 static 가중치를 이 파일 안에 가둔다 */

#include "actor_phase1.h"
#include "policy_actor.h"


void actorPhase1Forward(const float *obs, float *action)
{
  actor_forward(obs, action);
}
