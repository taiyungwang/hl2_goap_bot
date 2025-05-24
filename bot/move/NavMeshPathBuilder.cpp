#include "NavMeshPathBuilder.h"

#include <nav_mesh/nav_area.h>

void NavMeshPathBuilder::build(Path& path, CNavArea *start) {
	if (start == nullptr) {
		return;
	}
	CNavArea::ClearSearchLists();
	CNavArea::MakeNewMarker();
	start->SetTotalCost(getHeuristicCost(start));
	start->SetPathLengthSoFar(0.0f);
	start->AddToOpenList();
	start->SetParent(nullptr);
	while (!CNavArea::IsOpenListEmpty()) {
		CNavArea *area = CNavArea::PopOpenList();
		if (foundGoal(area)) {
			for (; area != nullptr; area = area->GetParent()) {
				path.emplace_back(area->GetID(), area->GetParentHow());
			}
			return;
		}
		for (int i = 0; i < NUM_DIRECTIONS; i++) {
			auto &connections = *area->GetAdjacentAreas(static_cast<NavDirType>(i));
			FOR_EACH_VEC(connections, j)
			{
				float length = connections[j].length;
				if ( area->GetAttributes() & NAV_MESH_CROUCH )
				{
					length *= 20.0f;
				}
				// if this is a "jump" area, add penalty
				if ( area->GetAttributes() & NAV_MESH_JUMP )
				{
					length *= 5.0f;
				}
				considerNeighbor(area, connections[j].area,
						static_cast<NavTraverseType>(i), length);
			}
		}
		for (int i = CNavLadder::LADDER_UP; i < CNavLadder::NUM_LADDER_DIRECTIONS; i++) {
			auto &connections = *area->GetLadders(static_cast<CNavLadder::LadderDirectionType>(i));
			FOR_EACH_VEC(connections, j)
			{
				const auto ladder = connections[j].ladder;
				const auto how = static_cast<NavTraverseType>(i + GO_LADDER_UP);
				if (i > CNavLadder::LADDER_UP) {
					float length = ladder->m_length + area->GetCenter().DistTo(ladder->m_top);
					if (ladder->m_bottomArea != nullptr) {
						length += ladder->m_bottomArea->GetCenter().DistTo(ladder->m_bottom);
					}
					considerNeighbor(area, ladder->m_bottomArea, how, length);
				} else {
					// ignore m_topBehindArea
					for (CNavArea *neighbor: {
						ladder->m_topForwardArea,
						ladder->m_topLeftArea,
						ladder->m_topRightArea
					}) {
						float length = ladder->m_length + area->GetCenter().DistTo(ladder->m_bottom);
						if (neighbor != nullptr) {
							length += neighbor->GetCenter().DistTo(ladder->m_top);
						}
						considerNeighbor(area, neighbor, how, length);
					}
				}
			}
		}
		auto connections = area->GetElevatorAreas();
		FOR_EACH_VEC(connections, i) {
			const auto neighbor = connections[i].area;
			considerNeighbor(area, neighbor,
					neighbor->GetCenter().z > area->GetCenter().z ? GO_ELEVATOR_UP : GO_ELEVATOR_DOWN,
					connections[i].length);
		}
		area->AddToClosedList();
	}
}

float NavMeshPathBuilder::getCost(CNavArea *area) const {
	return area->GetDanger(team);
}

bool NavMeshPathBuilder::shouldSearch(CNavArea *area) const {
	return !area->IsBlocked(team);
}

void NavMeshPathBuilder::considerNeighbor(CNavArea *area, CNavArea *neighbor,
		const NavTraverseType how, const float length) const {
	if (neighbor == nullptr) {
		return;
	}
	const float tentativePathLength = area->GetPathLengthSoFar() + length + getCost(neighbor);
	const bool isNeighborOpen = neighbor->IsOpen();
	if (!shouldSearch(neighbor)
		|| tentativePathLength >= (isNeighborOpen || neighbor->IsClosed() ? neighbor->GetPathLengthSoFar() : INFINITY)) {
		return;
	}
	neighbor->SetParent(area, how);
	neighbor->SetPathLengthSoFar(tentativePathLength);
	neighbor->SetTotalCost(tentativePathLength + getHeuristicCost(neighbor));
	if (!isNeighborOpen) {
		neighbor->AddToOpenList();
	} else {
		neighbor->UpdateOnOpenList();
	}
}
