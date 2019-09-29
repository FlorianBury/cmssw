#ifndef SimPPS_RPDigiProducer_RPSimTypes_H
#define SimPPS_RPDigiProducer_RPSimTypes_H

#include <map>
#include <vector>
#include <set>

#include "SimPPS/RPDigiProducer/interface/RPEnergyDepositUnit.h"
#include "SimPPS/RPDigiProducer/interface/RPSignalPoint.h"

typedef uint32_t RPDetId;

namespace simromanpot {
  typedef std::map<unsigned short, double> strip_charge_map;
  typedef std::vector<RPSignalPoint> charge_induced_on_surface;
  typedef std::vector<RPEnergyDepositUnit> energy_path_distribution;
  typedef std::set<short, std::less<short> > HitsContainer;
  typedef std::set<short, std::less<short> >::const_iterator HitsContainerIter;
  typedef std::set<short, std::less<short> > TriggerContainer;
  typedef std::set<short, std::less<short> >::const_iterator TriggerContainerIter;
  typedef std::vector<std::vector<std::pair<int, double> > >
      DigiPrimaryMapType;  //for each digi in the output the vector of the number of PSimHit and its weight
  typedef std::vector<std::pair<int, double> >
      SingleDigiPrimaryMapType;  //for one digi in the output the vector of the number of PSimHit and its weight
  typedef std::vector<std::vector<std::pair<int, double> > >
      TriggerPrimaryMapType;  //for each digi in the output the vector of the number of PSimHit and its weight
  typedef std::map<unsigned short, std::vector<std::pair<int, double> > >
      strip_charge_map_links_type;  //for each strip the indeces of PSimHit and amounts of charge generated by it is given
  typedef std::map<unsigned short, std::map<int, double> >
      TriggerContainerLinkMap;  //for each strip the indeces of PSimHit and amounts of charge generated by it is given
}  // namespace simromanpot

#endif  //SimPPS_RPDigiProducer_RPSimTypes_H
