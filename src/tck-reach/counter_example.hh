/*
 * This file is a part of the TChecker project.
 *
 * See files AUTHORS and LICENSE for copyright details.
 *
 */

#ifndef TCHECKER_TCK_REACH_COUNTER_EXAMPLE_HH
#define TCHECKER_TCK_REACH_COUNTER_EXAMPLE_HH

#include <memory>
#include <vector>

#include "tchecker/algorithms/path/algorithm.hh"
#include "tchecker/refzg/path.hh"
#include "tchecker/refzg/refzg.hh"
#include "tchecker/syncprod/vedge.hh"
#include "tchecker/syncprod/vloc.hh"
#include "tchecker/zg/path.hh"
#include "tchecker/zg/zg.hh"

/*!
 \file counter_example.hh
 \brief Generation of counter examples
*/

namespace tchecker {

namespace tck_reach {

/*!
 \brief Check if a node is initial
 \tparam GRAPH : type of graph
 \param n : a node
 \return true if n is an initial node, false otherwise
*/
template <class GRAPH> bool initial_node(typename GRAPH::node_sptr_t const & n) { return n->initial(); }

/*!
 \brief Check is a node is final
 \tparam GRAPH : type of graph
 \param n : a node
 \return true if n is a final node, false otherwise
*/
template <class GRAPH> bool final_node(typename GRAPH::node_sptr_t const & n) { return n->final(); }

/*!
 \brief True predicate over edges
 \tparam GRAPH : type of graph
 \param e : an edge
 \return true
*/
template <class GRAPH> bool true_edge(typename GRAPH::edge_sptr_t const & e) { return true; }

/*!
 \brief Compute a counter example over the zone graph
 \tparam GRAPH : type of graph, see tchecker::algorithms::path::finite::algorithm_t for requirements
 \param g : a graph over the zone graph (reachability graph, subsumption graph, etc)
 \return a finite path from an initial node of g to a final node of g
 */
template <class GRAPH> tchecker::zg::path::concrete::finite_path_t * counter_example_zg(GRAPH const & g)
{
  std::shared_ptr<tchecker::zg::zg_t> zg{tchecker::zg::factory(
      g.zg().system_ptr(), g.zg().sharing_type(), tchecker::zg::STANDARD_SEMANTICS, tchecker::zg::NO_EXTRAPOLATION, 128, 128)};

  // compute sequence of edges from initial to final node in g
  tchecker::algorithms::path::finite::algorithm_t<GRAPH> algorithm;

  std::vector<typename GRAPH::edge_sptr_t> seq =
      algorithm.run(g, &tchecker::tck_reach::initial_node<GRAPH>, &tchecker::tck_reach::final_node<GRAPH>,
                    &tchecker::tck_reach::true_edge<GRAPH>);

  if (seq.empty())
    return new tchecker::zg::path::concrete::finite_path_t{zg, 1};

  // Extract sequence of vedges
  std::vector<tchecker::const_vedge_sptr_t> vedge_seq;
  for (typename GRAPH::edge_sptr_t const & e : seq)
    vedge_seq.push_back(e->vedge_ptr());

  // Get the corresponding run in a zone graph with standard semantics and no extrapolation
  tchecker::vloc_t const & initial_vloc = g.edge_src(seq[0])->state().vloc();
  std::unique_ptr<tchecker::zg::path::symbolic::finite_path_t> zg_path{
      tchecker::zg::path::symbolic::compute(zg, initial_vloc, vedge_seq)};
  if (!zg_path->empty()) {
    zg_path->first()->initial(true);
    zg_path->last()->final(true);
  }

  // Compute counter-exemple from zone graph path
  tchecker::zg::path::concrete::finite_path_t * cex = tchecker::zg::path::concrete::compute(*zg_path);

  return cex;
}

/*!
 \brief Compute a symbolic counter example over the zone graph with reference clocks
 \tparam GRAPH : type of graph, see tchecker::algorithms::path::finite::algorithm_t for requirements
 \tparam CEX : type of counter example, should inherit from tchecker::refzg::path::finite_path_t
 \param g : a graph over the zone graph (reachability graph, subsumption graph, etc)
 \return a finite path from an initial node of g to a final node of g
 */
template <class GRAPH, class CEX> CEX * symbolic_counter_example_refzg(GRAPH const & g)
{
  std::shared_ptr<tchecker::refzg::refzg_t> refzg{
      tchecker::refzg::factory(g.refzg().system_ptr(), g.refzg().sharing_type(), tchecker::refzg::PROCESS_REFERENCE_CLOCKS,
                               tchecker::refzg::STANDARD_SEMANTICS, g.refzg().spread(), 128, 128)};

  // compute sequence of edges from initial to final node in g
  tchecker::algorithms::path::finite::algorithm_t<GRAPH> algorithm;

  std::vector<typename GRAPH::edge_sptr_t> seq =
      algorithm.run(g, &tchecker::tck_reach::initial_node<GRAPH>, &tchecker::tck_reach::final_node<GRAPH>,
                    &tchecker::tck_reach::true_edge<GRAPH>);

  if (seq.empty())
    return new CEX{refzg};

  // Extract sequence of vedges
  std::vector<tchecker::const_vedge_sptr_t> vedge_seq;
  for (typename GRAPH::edge_sptr_t const & e : seq)
    vedge_seq.push_back(e->vedge_ptr());

  // Get the corresponding run in a zone graph with standard semantics and no extrapolation
  tchecker::vloc_t const & initial_vloc = g.edge_src(seq[0])->state().vloc();
  CEX * cex = tchecker::refzg::path::compute_symbolic_run(refzg, initial_vloc, vedge_seq);
  if (!cex->empty()) {
    cex->first()->initial(true);
    cex->last()->final(true);
  }

  return cex;
}

} // namespace tck_reach

} // namespace tchecker

#endif // TCHECKER_TCK_REACH_COUNTER_EXAMPLE_HH
