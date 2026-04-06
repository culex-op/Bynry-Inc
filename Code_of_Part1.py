@app.route('/api/products', methods=['POST']) # [cite: 5]
def new_prod():
    d = request.json # Parse payload safely
    try:
        # Init prod without w_id to allow many-to-many
        p = Product(
            name=d.get('name'), 
            sku=d.get('sku'), 
            price=d.get('price')
        )
        db.session.add(p)
        db.session.flush() # Get p.id without committing to DB yet
        
        # Link prod to warehouse via join table
        i = Inventory(
            p_id=p.id, 
            w_id=d.get('warehouse_id'), 
            qty=d.get('initial_quantity', 0)
        )
        db.session.add(i)
        db.session.commit() # Atomic commit for both p and i
        
        return {"msg": "ok", "p_id": p.id}, 201
    except Exception as e:
        db.session.rollback() # Revert everything if either insert fails
        return {"err": "bad req"}, 400
