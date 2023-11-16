#include <vector>
#include <queue>
#include <unordered_map>
#include <unordered_set>
#include <chrono>
#include <iostream>
//#include <stdlib.h>
//#include <crtdbg.h>

#include "../platform.h" // This file will make exporting DLL symbols simpler for students.
#include "../Framework/TileSystem/TileMapUtility.h"
#include "../PriorityQueue.h"
#pragma once

//#define _CRTDBG_MAP_ALLOC
#define HEURISTIC_WEIGHT 1.2	//heuristic weight to match the example program


namespace ufl_cap4053
{
	namespace searches
	{
		class PathSearch
		{
		private:
			class SearchNode {
				ufl_cap4053::Tile* vertex;
				std::vector<SearchNode*> neighbors;
			public:
				SearchNode(ufl_cap4053::Tile* t);
				void addNeighbor(SearchNode* n);
				ufl_cap4053::Tile* getTile() const;
				std::vector<SearchNode*>& getNeighbors();
			};

			class PlannerNode {
				friend class PathSearch;
				SearchNode* vertex;
				PlannerNode* parent;
				//float heuristic;
				float givenCost;
				float totalCost;
			public:
				PlannerNode(SearchNode* v, PlannerNode* p, float heuristic, float unitLength);
				PlannerNode(SearchNode* v, float heuristic);
				PlannerNode* getParent() const;
				SearchNode* getNode() const;
				ufl_cap4053::Tile* getTile() const;
				//float getHeuristic() const;
				float getCost() const;
				float getTotalCost() const;
			};

			static bool isGreaterThan(PlannerNode* const& lhs, PlannerNode* const& rhs);

			std::unordered_map<ufl_cap4053::Tile*, SearchNode*> searchGraph;
			std::unordered_map<SearchNode*, PlannerNode*> queuedNodes;
			std::unordered_set<SearchNode*> visited;
			//std::queue<PlannerNode*> searchQueue;
			ufl_cap4053::PriorityQueue<PlannerNode*> searchQueue;
			std::vector<Tile const*> finalPath;

			ufl_cap4053::TileMap* tileMap;

			int beginCol;
			int beginRow;
			int endRow;
			int endCol;

			float endY;
			float endX;
			float stepSize;		//distance between 2 adjacent tiles 

			//named to avoid confusion with function parameters

			bool done;

			void buildSearchGraph();
			void searchIteration();
			void searchFinalize(PlannerNode* endpoint);
			bool areAdjacent(const Tile* lhs, const Tile* rhs);
			//private helper functions

		// CLASS DECLARATION GOES HERE
			public:
				DLLEXPORT PathSearch(); // EX: DLLEXPORT required for public methods - see platform.h
				DLLEXPORT ~PathSearch();
				DLLEXPORT void load(ufl_cap4053::TileMap* _tilemap);
				DLLEXPORT void initialize(int startRow, int startCol, int goalRow, int goalCol);
				DLLEXPORT void update(long timeslice);
				DLLEXPORT void shutdown();
				DLLEXPORT void unload();
				DLLEXPORT bool isDone() const;
				DLLEXPORT std::vector<ufl_cap4053::Tile const*> const getSolution() const;
		};
	}
}  // close namespace ufl_cap4053::searches
