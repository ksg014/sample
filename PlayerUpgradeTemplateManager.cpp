#include "PlayerUpgradeTemplateManager.h" 
#include "ServerUtil.h"
#include "sqlite_orm/sqlite_orm.h"
#include "spdlog/spdlog.h"

PlayerUpgradeTemplateManager::PlayerUpgradeTemplateManager()
{
}

PlayerUpgradeTemplateManager::~PlayerUpgradeTemplateManager()
{
}

bool PlayerUpgradeTemplateManager::Init()
{
	std::string dataDBPath = Util::GetDataDBPath();

	// PlayerUpgradeStatTemplate
	{
		auto storage = sqlite_orm::make_storage(dataDBPath, sqlite_orm::make_table("PlayerUpgradeStatTemplate"
			, sqlite_orm::make_column("PlayerUpgradeTID", &PlayerUpgradeStatTemplate::m_playerUpgradeTID)
			, sqlite_orm::make_column("Level", &PlayerUpgradeStatTemplate::m_level)
			, sqlite_orm::make_column("Value", &PlayerUpgradeStatTemplate::m_value)
			, sqlite_orm::make_column("UpgradeByteCost", &PlayerUpgradeStatTemplate::m_upgradeByteCost)
			, sqlite_orm::make_column("RequireCharacterLevel", &PlayerUpgradeStatTemplate::m_requireCharacterLevel)
		));

		try {
			// 비용 누적용, <PlayerUpgradeTID, < Level, UpgradeByteCost 누적 > >
			std::map<TID, std::pair<int, int>> upgradeStatCostCumulative;

			auto playerUpgradeStatTemplateALL = storage.get_all<PlayerUpgradeStatTemplate>();
			for (auto& playerUpgradeStatTemplate : playerUpgradeStatTemplateALL)
			{
				auto& cumulativePair = upgradeStatCostCumulative[playerUpgradeStatTemplate.m_playerUpgradeTID];
				if (cumulativePair.first + 1 != playerUpgradeStatTemplate.m_level)
				{
					SPDLOG_ERROR("PlayerUpgradeStatTemplate need to align level. TID:{0}, Level:{1}", playerUpgradeStatTemplate.m_playerUpgradeTID, playerUpgradeStatTemplate.m_level);
					return false;
				}

				cumulativePair.first = playerUpgradeStatTemplate.m_level;
				cumulativePair.second += playerUpgradeStatTemplate.m_upgradeByteCost;

				playerUpgradeStatTemplate.m_cumulativeUpgradeByteCost = cumulativePair.second;

				if (m_playerUpgradeStatTemplate[playerUpgradeStatTemplate.m_playerUpgradeTID].insert({playerUpgradeStatTemplate.m_level, playerUpgradeStatTemplate}).second == false)
				{
					SPDLOG_ERROR("PlayerUpgradeStatTemplate PlayerUpgradeTID, Level Duplicated. TID:{0}, Level:{1}", playerUpgradeStatTemplate.m_playerUpgradeTID, playerUpgradeStatTemplate.m_level);
					return false;
				}
			}
		}
		catch (std::system_error e)
		{
			SPDLOG_ERROR("{}", e.what());
			return false;
		}
	}

	// PlayerUpgradeTemplate
	{
		auto storage = sqlite_orm::make_storage(dataDBPath, sqlite_orm::make_table("PlayerUpgradeTemplate"
			, sqlite_orm::make_column("TemplateID", &PlayerUpgradeTemplate::m_templateID)
			, sqlite_orm::make_column("StatType", &PlayerUpgradeTemplate::SetStatType, &PlayerUpgradeTemplate::GetStatType)
			, sqlite_orm::make_column("StatSubType", &PlayerUpgradeTemplate::SetStatSubType, &PlayerUpgradeTemplate::GetStatSubType)
			, sqlite_orm::make_column("UnlockPlayerUpgradeTID", &PlayerUpgradeTemplate::m_unlockPlayerUpgradeTID)
			, sqlite_orm::make_column("UnlockPlayerUpgradeLevel", &PlayerUpgradeTemplate::m_unlockPlayerUpgradeLevel)
		));

		try {
			auto playerUpgradeTemplateALL = storage.get_all<PlayerUpgradeTemplate>();
			for (auto& playerUpgradeTemplate : playerUpgradeTemplateALL)
			{
				auto it = m_playerUpgradeStatTemplate.find(playerUpgradeTemplate.m_templateID);
				if (it == m_playerUpgradeStatTemplate.end())
				{
					SPDLOG_ERROR("PlayerUpgradeStatTemplate template is not exist. TID:{}", playerUpgradeTemplate.m_templateID);
					return false;
				}
				else
				{
					playerUpgradeTemplate.m_playerUpgradeStatTemplate = &it->second;

					for (auto& statPair : *playerUpgradeTemplate.m_playerUpgradeStatTemplate)
					{
						auto& statTemplate = statPair.second;

						switch (playerUpgradeTemplate.m_statSubType)
						{
						case StatSubType::Value_Up:
							statTemplate.m_value_Up = statTemplate.m_value;
							break;
						case StatSubType::Value_Down:
							statTemplate.m_value_Down = statTemplate.m_value;
							break;
						case StatSubType::Value_RatioUp:
							statTemplate.m_value_RatioUp = statTemplate.m_value;
							break;
						case StatSubType::Value_RatioDown:
							statTemplate.m_value_RatioDown = statTemplate.m_value;
							break;
						}
					}
				}

				if (m_playerUpgradeTemplate.insert({ playerUpgradeTemplate.m_templateID, playerUpgradeTemplate }).second == false)
				{
					SPDLOG_ERROR("PlayerUpgradeTemplate templateID Duplicated. TID:{}", playerUpgradeTemplate.m_templateID);
					return false;
				}
			}
		}
		catch (std::system_error e)
		{
			SPDLOG_ERROR("{}", e.what());
			return false;
		}
	}

	return true;
}

const PlayerUpgradeTemplate* PlayerUpgradeTemplateManager::GetPlayerUpgradeTemplate(TID tid)
{
	if (auto it = m_playerUpgradeTemplate.find(tid); it != m_playerUpgradeTemplate.end())
		return &it->second;

	return nullptr;
}
