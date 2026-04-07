#include <iostream>
#include <vector>
#include <string>
#include <nlohmann/json.hpp> // Standard C++ JSON lib

using json = nlohmann::json;
using namespace std;

// Structs mapping to our joined DB query results
struct Sup { int id; string nm, em; };
struct Inv { int p_id; string nm, sku, typ; int w_id; string w_nm; int qty, vlc; Sup s; };

json get_low_stock_alerts(int c_id) {
    json res;
    res["alerts"] = json::array();
    
    // Edge Case: Validate input
    if (c_id <= 0) return {{"err", "invalid c_id"}}; 
    
    // db call fetches inventory joined with supplier/warehouse data
    // only returns rows where vlc > 0 (recent sales activity rule)
    vector<Inv> data = db.get_inv(c_id); 
    
    for (auto& r : data) {
        // Business Rule: Threshold varies by product type
        int thr = (r.typ == "fast_moving") ? 50 : 20; 
        
        if (r.qty <= thr) {
            // Edge Case: Prevent division by zero if vlc is exactly 0 
            // even though db filtered it, defense in depth is good practice
            int dts = (r.vlc > 0) ? (r.qty / r.vlc) : 999; 
            
            res["alerts"].push_back({
                {"product_id", r.p_id},
                {"product_name", r.nm},
                {"sku", r.sku},
                {"warehouse_id", r.w_id},
                {"warehouse_name", r.w_nm},
                {"current_stock", r.qty},
                {"threshold", thr},
                {"days_until_stockout", dts},
                {"supplier", {
                    {"id", r.s.id},
                    {"name", r.s.nm},
                    {"contact_email", r.s.em.empty() ? "N/A" : r.s.em} // Edge Case: Missing email
                }}
            });
        }
    }
    
    res["total_alerts"] = res["alerts"].size();
    return res;
}
