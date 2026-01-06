#include "PartyComponent.h"
#include "Party.h"
#include "GamePlayer.h"
#include "GamePlayerContainer.h"
#include "Protocol/Packet.h"
#include "PartyManager.h"

PartyComponent::PartyComponent(std::shared_ptr<BaseEntity> baseEntity) : BaseComponent(baseEntity)
{
	m_party = nullptr;
}

PartyComponent::~PartyComponent()
{
	Release();
}

ErrorCode PartyComponent::PartyInviteReq(CUID targetCUID, const std::string& targetName)
{
	if (m_party == nullptr)
		return ErrorCode::NotFoundParty;

	auto leader = m_party->GetLeader();
	if (leader == nullptr)
		return ErrorCode::NotFoundParty;

	auto player = GetGamePlayer();
	if (player == nullptr)
		return ErrorCode::NotFoundCharacter;

	if (player->GetCharacterUID() != leader->GetCharacterUID())
		return ErrorCode::WrongRequest;

	if (player->GetCharacterUID() == targetCUID)
		return ErrorCode::WrongRequest;

	if (m_party->GetMemberSize() > g_partyMaxMemberCount)
		return ErrorCode::PartyMemberCountIsMax;

	std::shared_ptr<GamePlayer> targetPlayer = nullptr;
	
	if (targetCUID > 0)
	{
		targetPlayer = GamePlayerContainer::GetInstance()->GetGamePlayerByCharacterUID(targetCUID);
	}
	else if (targetName.size() > 0)
	{
		targetPlayer = GamePlayerContainer::GetInstance()->GetGamePlayerByCharacterName(targetName);
	}

	if (targetPlayer == nullptr)
		return ErrorCode::NotFoundCharacter;

	targetCUID = targetPlayer->GetCharacterUID();

	auto targetPartyComponent = targetPlayer->GetComponent<PartyComponent>();
	if (targetPartyComponent == nullptr)
		return ErrorCode::NotFoundCharacter;

	if (targetPartyComponent->GetParty() != nullptr)
		return ErrorCode::AlreadyJoinParty;

	if (m_party->CheckInviteReqCool(targetCUID) == false)
		return ErrorCode::PartyInviteReqCool;

	auto expireTick = m_party->AddInviteReqList(targetCUID);

	PacketPartyInviteAnswerReq req;
	req.m_partyID = m_party->GetPartyID();
	req.m_leaderName = player->GetCharacterName();
	req.m_expireTick = expireTick;
	 
	targetPlayer->Send(req);

	PacketPartyInviteAck partyInviteAck;
	player->Send(partyInviteAck);

	return ErrorCode::None;
}

ErrorCode PartyComponent::PartyTransferLeaderReq(CUID target)
{
	if (m_party == nullptr)
		return ErrorCode::NotFoundParty;

	auto leader = m_party->GetLeader();
	if (leader == nullptr)
		return ErrorCode::NotFoundParty;

	auto player = GetGamePlayer();
	if (player == nullptr)
		return ErrorCode::NotFoundCharacter;

	if (player->GetCharacterUID() != leader->GetCharacterUID())
		return ErrorCode::WrongRequest;

	if (player->GetCharacterUID() == target)
		return ErrorCode::WrongRequest;

	return m_party->TransferLeader(target);
}

ErrorCode PartyComponent::PartyLeaveReq()
{
	if (m_party == nullptr)
		return ErrorCode::NotFoundParty;

	auto leader = m_party->GetLeader();
	if (leader == nullptr)
		return ErrorCode::NotFoundParty;

	auto player = GetGamePlayer();
	if (player == nullptr)
		return ErrorCode::NotFoundCharacter;

	auto err = m_party->LeaveMember(player->GetCharacterUID());
	if (err != ErrorCode::None)
		return err;

	PacketPartyLeaveAck partyLeaveAck;
	player->Send(partyLeaveAck);

	return ErrorCode::None;
}

ErrorCode PartyComponent::PartyExpelReq(CUID target)
{
	if (m_party == nullptr)
		return ErrorCode::NotFoundParty;

	auto leader = m_party->GetLeader();
	if (leader == nullptr)
		return ErrorCode::NotFoundParty;

	auto player = GetGamePlayer();
	if (player == nullptr)
		return ErrorCode::NotFoundCharacter;

	if (player->GetCharacterUID() != leader->GetCharacterUID())
		return ErrorCode::WrongRequest;

	if (player->GetCharacterUID() == target)
		return ErrorCode::WrongRequest;

	return m_party->ExpelMember(target);
}

ErrorCode PartyComponent::PartyJoinAcceptAck(CUID target)
{
	if (m_party == nullptr)
		return ErrorCode::NotFoundParty;

	auto leader = m_party->GetLeader();
	if (leader == nullptr)
		return ErrorCode::NotFoundParty;

	auto player = GetGamePlayer();
	if (player == nullptr)
		return ErrorCode::NotFoundCharacter;

	if (player->GetCharacterUID() != leader->GetCharacterUID())
		return ErrorCode::WrongRequest;

	if (player->GetCharacterUID() == target)
		return ErrorCode::WrongRequest;

	return m_party->JoinAccept(target);
}

void PartyComponent::Release()
{
	m_party = nullptr;
}

void PartyComponent::Logout()
{
	if (m_party == nullptr)
		return;

	std::shared_ptr<GamePlayer> gamePlayer = GetGamePlayer();
	if (gamePlayer == nullptr)
		return;

	m_party->Logout(gamePlayer->GetCharacterUID());
	m_party = nullptr;
}

void PartyComponent::SetParty(std::shared_ptr<Party> party)
{
	m_party = party;
}
