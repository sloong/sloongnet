#ifndef SERVERMANAGE_H
#define SERVERMANAGE_H

#include "main.h"
#include "configuation.h"
#include "DataTransPackage.h"
#include "IData.h"
namespace Sloong
{

	struct ServerItem
	{
		void Active() { ActiveTime = time(NULL); }
		string Address;
		int Port;
		int ActiveTime;
		string UUID;
		int Template_ID;
		ModuleType Type;
	};

	struct TemplateItem
	{
		TemplateItem() {}
		TemplateItem(TemplateInfo info) {
			ID = info.id;
			Name = info.name;
			Note = info.note;
			Configuation = info.configuation;
			Replicas = info.replicas;
			Type = (ModuleType)info.type;
		}
		TemplateInfo ToTemplateInfo() {
			TemplateInfo info;
			info.configuation = this->Configuation;
			info.id = this->ID;
			info.name = this->Name;
			info.note = this->Note;
			info.replicas = this->Replicas;
			info.type = this->Type;
			return info;
		}
		int ID;
		string Name;
		string Note;
		int Replicas;
		ModuleType Type;
		string Configuation;
		list_ex<string> Created;

	};


	class CServerManage
	{
	public:
		CResult Initialize( int template_id );

		bool RegisterServerHandler(Functions func, string sender, SmartPackage pack);
		bool GetTemplateListHandler(Functions func, string sender, SmartPackage pack);
		bool SetTemplateConfigHandler(Functions func, string sender, SmartPackage pack);
		bool GetServerListHandler(Functions func, string sender, SmartPackage pack);

		/// 由于初始化太早，无法在initialize时获取。只能由control_server手动设置
		void SetLog(CLog* log) { m_pLog = log; }

	private:
		int SearchNeedCreateTemplate();

	private:
		unique_ptr<CConfiguation>	m_pAllConfig = make_unique<CConfiguation>();
		CLog* m_pLog = nullptr;
		map_ex<string, ServerItem>	m_oServerList;
		map_ex<int, TemplateItem>	m_oTemplateList;
	};
}
#endif
