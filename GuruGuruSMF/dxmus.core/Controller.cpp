#include "stdafx.h"
#include "Controller.h"

namespace GuruGuruSmf{ namespace Dxmus{

Controller* Controller::instance = 0;

// コンストラクタ
Controller::Controller()
{
	performancePlayer = new PerformancePlayer();
	port = new Port();
}

// デストラクタ
Controller::~Controller()
{
	delete performancePlayer;
	delete port;
}

// インスタンス取得
Controller* Controller::GetInstance()
{
	if(instance == 0){
		instance = new Controller();
	}
	return instance;
}

// インスタンス開放
void Controller::FreeInstance()
{
	delete instance;
	instance = 0;
}

}}
