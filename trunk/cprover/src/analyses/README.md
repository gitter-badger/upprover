\ingroup module_hidden
\defgroup analyses analyses

# Folder analyses

This contains the abstract interpretation framework `ai.h` and several
static analyses that instantiate it.

\section analyses-frameworks Frameworks:

\subsection analyses-ait Abstract interpreter framework (ait)

To be documented.

\subsection analyses-static-analysist Old Abstract interpreter framework (static_analysist)

This is obsolete.

\subsection analyses-flow-insensitive-analysis Flow-insensitive analysis (flow_insensitive_analysist)

Framework for flow-insensitive analyses. Maintains a single global abstract
value which instructions are invited to transform in turn. Unsound (terminates
too early) if
(a) there are two or more instructions that incrementally reach a fixed point,
for example by walking a chain of pointers and updating a points-to set, but
(b) those instructions are separated by instructions that don't update the
abstract value (for example, SKIP instructions). Therefore, not recommended for
new code.

Only current user in-tree is \ref value_set_analysis_fit and its close
relatives, \ref value_set_analysis_fivrt and \ref value_set_analysis_fivrnst

\section analyses-specific-analyses Specific analyses:

\subsection analyses-call-graph Call graph and associated helpers (call_grapht)

A [https://en.wikipedia.org/wiki/Call_graph](call graph) for a GOTO model or
GOTO functions collection. \ref call_grapht implements a basic call graph, but
can also export the graph in \ref grapht format, which permits more advanced
graph algorithms to be applied; see \ref call_graph_helpers.h for functions
that work with the \ref grapht representation.

\subsection analyses-dominator Dominator analysis (cfg_dominators_templatet)

To be documented.

\subsection analyses-constant-propagation Constant propagation (\ref constant_propagator_ait)

A simple, unsound constant propagator. Replaces RHS symbol expressions (variable
reads) with their values when they appear to have a unique value at a particular
program point. Unsound with respect to pointer operations on the left-hand side
of assignments.

\subsection analyses-taint Taint analysis (custom_bitvector_analysist)

To be documented.

\subsection analyses-dependence-graph Data- and control-dependence analysis (dependence_grapht)

To be documented.

\subsection analyses-dirtyt Address-taken lvalue analysis (dirtyt)

To be documented.

\subsection analyses-const-cast-removal const_cast removal analysis (does_remove_constt)

To be documented.

\subsection analyses-escape Escape analysis (escape_analysist)

To be documented.

\subsection analyses-global-may-alias Global may-alias analysis (global_may_aliast)

To be documented.

\subsection analyses-rwt Read-write range analysis (goto_rwt)

To be documented.

\subsection analyses-invariant-propagation Invariant propagation (invariant_propagationt)

To be documented.

\subsection analyses-is-threaded Multithreaded program detection (is_threadedt)

To be documented.

\subsection analyses-pointer-classification Pointer classification analysis (is-heap-pointer, might-be-null, etc -- local_bitvector_analysist)

To be documented.

\subsection analyses-cfg Control-flow graph (local_cfgt)

To be documented.

\subsection analyses-local-may-alias Local may-alias analysis (local_may_aliast)

To be documented.

\subsection analyses-safe-dereference Safe dereference analysis (local_safe_pointerst)

To be documented.

\subsection analyses-locals Address-taken locals analysis (localst)

To be documented.

\subsection analyses-natural-loop Natural loop analysis (natural_loops_templatet)

To be documented.

\subsection analyses-reaching-definitions Reaching definitions (reaching_definitions_analysist)

To be documented.

\subsection analyses-uncaught-exceptions Uncaught exceptions analysis (uncaught_exceptions_domaint)

To be documented.

\subsection analyses-uninitialized-locals Uninitialized locals analysis (uninitialized_analysist)

To be documented.

\section analyses-transformations Transformations (arguably in the wrong directory):

\subsection analyses-goto-checkt Pointer / overflow / other check insertion (goto_checkt)

To be documented.

\subsection analyses-interval-analysist Integer interval analysis (interval_analysist) -- both an analysis and a transformation

To be documented.
