#include "ProgramEvent.h"


using namespace Sloong::Events;

Sloong::Events::CProgramStartEvent::~CProgramStartEvent()
{
	SAFE_DELETE_ARR(m_pParams);
}
