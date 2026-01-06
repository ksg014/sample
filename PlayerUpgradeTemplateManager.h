#pragma once
#include <map>
#include "Singleton.h"
#include "Define/DefineType.h" 
#include "Define/DefineEnum.h"

struct PlayerUpgradeStatTemplate
{
	TID m_playerUpgradeTID = 0;
	int m_level = 0;
	int m_value = 0;
	int m_upgradeByteCost = 0;
	int m_requireCharacterLevel = 0;

	int64_t m_cumulativeUpgradeByteCost = 0;	// 레벨별 코스트 누적량
	int m_value_Up = 0;
	int m_value_Down = 0;
	int m_value_RatioUp = 0;
	int m_value_RatioDown = 0;
};

struct PlayerUpgradeTemplate
{
	void SetStatType(int statType) { m_statType = (StatType)statType; }
	int GetStatType() const { return (int)m_statType; }

	void SetStatSubType(int statSubType) { m_statSubType = (StatSubType)statSubType; }
	int GetStatSubType() const { return (int)m_statSubType; }

	TID m_templateID = 0;
	StatType m_statType = StatType::None;
	StatSubType m_statSubType = StatSubType::None;
	TID m_unlockPlayerUpgradeTID = 0;
	int m_unlockPlayerUpgradeLevel = 0;

	// key: PlayerUpgradeStatTemplate::m_level
	std::map<int, PlayerUpgradeStatTemplate>* m_playerUpgradeStatTemplate = nullptr;
};

class PlayerUpgradeTemplateManager : public Singleton<PlayerUpgradeTemplateManager>
{
public:
	PlayerUpgradeTemplateManager();
	virtual ~PlayerUpgradeTemplateManager();

	bool Init();

	const PlayerUpgradeTemplate* GetPlayerUpgradeTemplate(TID tid);

private:
	std::map<TID, PlayerUpgradeTemplate> m_playerUpgradeTemplate;
	std::map<TID, std::map<int/*Level*/, PlayerUpgradeStatTemplate>> m_playerUpgradeStatTemplate;
};