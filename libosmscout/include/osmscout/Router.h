#ifndef OSMSCOUT_ROUTER_H
#define OSMSCOUT_ROUTER_H

/*
  This source is part of the libosmscout library
  Copyright (C) 2012  Tim Teulings

  This library is free software; you can redistribute it and/or
  modify it under the terms of the GNU Lesser General Public
  License as published by the Free Software Foundation; either
  version 2.1 of the License, or (at your option) any later version.

  This library is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
  Lesser General Public License for more details.

  You should have received a copy of the GNU Lesser General Public
  License along with this library; if not, write to the Free Software
  Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA 02111-1307  USA
*/

#include <list>
#include <set>

#include <osmscout/TypeConfig.h>

#include <osmscout/RouteNode.h>

// Datafiles
#include <osmscout/WayDataFile.h>

// Fileoffset by Id index
#include <osmscout/WayIndex.h>

// Reverse index
#include <osmscout/NodeUseIndex.h>

// Routing
#include <osmscout/Route.h>
#include <osmscout/RoutingProfile.h>

#include <osmscout/util/Cache.h>

namespace osmscout {

  typedef DataFile<RouteNode> RouteNodeDataFile;

  /**
    Database instance initialisation parameter to influence the behaviour of the database
    instance.

    The following groups attributes are currently available:
    * cache sizes.
    */
  class OSMSCOUT_API RouterParameter
  {
  private:
    unsigned long wayIndexCacheSize;
    unsigned long wayCacheSize;

    bool          debugPerformance;

  public:
    RouterParameter();

    void SetWayIndexCacheSize(unsigned long wayIndexCacheSize);
    void SetWayCacheSize(unsigned long wayCacheSize);

    void SetDebugPerformance(bool debug);

    unsigned long GetWayIndexCacheSize() const;
    unsigned long GetWayCacheSize() const;

    bool IsDebugPerformance() const;
  };

  class OSMSCOUT_API Router
  {
  private:
    struct Follower
    {
      TypeId type;
      Id     wayId;
    };

    /**
     * A node in the routing graph, normally a node as part of a way
     */
    struct RNode
    {
      Id        nodeId;
      Id        wayId;
      double    currentCost;
      double    estimateCost;
      double    overallCost;
      Id        prev;

      RNode()
       : nodeId(0),
         wayId(0)
      {
      }

      RNode(Id nodeId,
            Id wayId,
            Id prev)
       : nodeId(nodeId),
         wayId(wayId),
         currentCost(0),
         estimateCost(0),
         overallCost(0),
         prev(prev)
      {
        // no code
      }

      inline bool operator==(const RNode& other)
      {
        return nodeId==other.nodeId;
      }

      inline bool operator<(const RNode& other) const
      {
        return nodeId<other.nodeId;
      }
    };

    struct RNodeCostCompare
    {
      inline bool operator()(const RNode& a, const RNode& b) const
      {
        if (a.overallCost==b.overallCost) {
         return a.nodeId<b.nodeId;
        }
        else {
          return a.overallCost<b.overallCost;
        }
      }
    };

    typedef std::set<RNode,RNodeCostCompare> OpenList;
    typedef OpenList::iterator               RNodeRef;

    struct RouteStep
    {
      Id wayId;
      Id nodeId;
    };

  private:
    bool                  isOpen;            //! true, if opened
    bool                  debugPerformance;

    std::string           path;              //! Path to the directory containing all files

    WayDataFile           wayDataFile;       //! Cached access to the 'ways.dat' file
    RouteNodeDataFile     routeNodeDataFile; //! Cached access to the 'route.dat' file

    TypeConfig            *typeConfig;       //! Type config for the currently opened map

  private:
    void GetClosestRouteNode(const WayRef& way,
                             Id nodeId,
                             RouteNodeRef& routeNode,
                             size_t& pos);
    void ResolveRNodesToList(const RNode& end,
                             const std::map<Id,RNode>& closeMap,
                             std::list<RNode>& nodes);
    void ResolveRNodesToRouteData(const std::list<RNode>& nodes,
                                  RouteData& route);


  public:
    Router(const RouterParameter& parameter);
    virtual ~Router();

    bool Open(const std::string& path);
    bool IsOpen() const;
    void Close();

    void FlushCache();

    TypeConfig* GetTypeConfig() const;

    bool CalculateRoute(const RoutingProfile& profile,
                        Id startWayId, Id startNodeId,
                        Id targetWayId, Id targetNodeId,
                        RouteData& route);

    bool TransformRouteDataToRouteDescription(const RouteData& data,
                                              RouteDescription& description);
    bool TransformRouteDataToWay(const RouteData& data,
                                 Way& way);

    void DumpStatistics();
  };
}

#endif