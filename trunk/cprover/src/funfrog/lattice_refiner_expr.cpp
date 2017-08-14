/* 
 * File:   lattice_refiner_expr.cpp
 * Author: karinek
 * 
 * Created on 13 August 2017, 20:09
 */

#include "lattice_refiner_expr.h"
#include <algorithm>


/*******************************************************************

 Function: lattice_refiner_exprt::get_refine_function

 Inputs: 

 Outputs: the next node in the lattice to refine the expression

 Purpose: 

\*******************************************************************/
lattice_refiner_modelt* lattice_refiner_exprt::get_refine_function() {
    if (m_is_SAT)
        return 0;

    return refine_data.front(); 
}


/*******************************************************************

 Function: lattice_refiner_exprt::process_SAT_result

 Inputs: 

 Outputs: 

 Purpose: if all reach to top - SAT, if SAT but not top, try the childs
 * of the current node. If reached to top (in all paths) the expression 
 * is removed from the refined data (since it cannot be refined).
 * 
 * Going down the lattice
 * 
 * SAT iff the current node had no childs and is SAT

\*******************************************************************/
void lattice_refiner_exprt::process_SAT_result() {
    if (refine_data.empty()) return;
    
    m_is_SAT = m_is_SAT && (refine_data.front()->childs.size() == 0);
    
    // Add the childs to the queue (if there is)
    for (auto it : refine_data.front()->childs) {
        if (refined_data_UNSAT.count(it) == 0) // If never check if (and got UNSAT)
            refine_data.push_back(it); // Adds it to the queue to check later on
    }
    refine_data.pop_front(); // Remove the node we used
}

/*******************************************************************

 Function: lattice_refiner_exprt::process_UNSAT_result

 Inputs: 

 Outputs: 

 Purpose: 
 * 
 * Going backward

\*******************************************************************/
void lattice_refiner_exprt::process_UNSAT_result() {
    if (refine_data.empty()) return;
    
    lattice_refiner_modelt *temp = refine_data.front();
    refined_data_UNSAT.insert(temp);
    refine_data.pop_front(); // Remove the node we used
    
    remove_dequed_data(temp);
}

/*******************************************************************

 Function: lattice_refiner_exprt::remove_dequed_data

 Inputs: 

 Outputs: 

 Purpose: 
 * 
 * Remove ancestors of UNSAT nodes (iff all its childs are UNSAT).
 * 
 * Not fully optimized

\*******************************************************************/
void lattice_refiner_exprt::remove_dequed_data(lattice_refiner_modelt *curr) {
    if (refine_data.empty()) return;
    
    // Per item in refine_data, if all its sons are in refined_data_UNSAT, remove it
    for(auto it = curr->ancestors.begin(); it != curr->ancestors.end() ; ++it) {
        if (is_all_childs_leads_to_UNSAT(*it)) // All my kids goes to UNSAT - remove too 
        {
            refine_data.erase(std::remove(refine_data.begin(), refine_data.end(), *it), refine_data.end()); // Remove the node we used
            
            refined_data_UNSAT.insert(*it);
            remove_dequed_data(*it); // Go up the tree
        }
    }
}

/*******************************************************************

 Function: lattice_refiner_exprt::is_all_childs_UNSAT

 Inputs: current location in the lattice

 Outputs: true, if all its childs leads eventually to UNSAT

 Purpose: 

\*******************************************************************/
bool lattice_refiner_exprt::is_all_childs_leads_to_UNSAT(lattice_refiner_modelt *curr) {
    assert(!refine_data.empty());
    
    // Per item in refine_data, if all its sons are in refined_data_UNSAT, remove it
    for(auto it = curr->childs.begin(); it != curr->childs.end() ; ++it) {
        if (refined_data_UNSAT.count(*it) == 0)
            return false;
    }
    
    return true;
}