// �X���[�v�֘A�̃��b�Z�[�W����������E�C���h�E

#pragma once

#include <Windows.h>

namespace GuruGuruSmf { namespace Core {

	/// <summary>
	/// �X���[�v�֘A�̃��b�Z�[�W����������E�C���h�E
	/// </summary>
	class PowerControlWindow
	{
	private:
		HWND _hWnd;
	public:
		PowerControlWindow();	// �R���X�g���N�^
		~PowerControlWindow();	// �f�X�g���N�^
		void createWindow();
		void MessageLoop();
		static void SetCallBackSuspend(void (*func)());
	};


}}

