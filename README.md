<html>
<body>
  
# Bynry-Inc

<p>Case Study</p>

# Part 1 : Code Review and Debugging

<li>Problem 1: The code exceutes two separate commits. If the Inventory creation fails(for example database timeout), the Product is already saved. In production , this creates orphaned products that cannot be properly tracked or sold.</li>

<li>Problem 2: When you put warehouse_id directly inside the Product table(or pass it to its consturctor) , you are dedicating a single column to store where that products lives. Because a single cell ina adatabse row can only hold one value , a product like "ABC" can only be assigned to "warehouse_1". This create one to many relationship: one warehouse can hold many products but each product is permanently locked to exactly one warehouse.</li>
<p>However it is given in problem statement that: Products can exist in multiple warehouses.</p>
<p>Products can be stored in multiple warehouses with different quantities.</p>
<p>If "BC" needs to be in both wareshouse_1 and warehouse_2 the 1 to many design would force you to create the completely duplicate "ABC" row just to assign it a new warehouse_id. This leads to data duplication and tracking nightmares.</p>
<li>The Solution is as follow : Many to Many , Product table should know anything about the warehouse.</li>
   <p>Product(product_id, name)</p>
   <p>Warehouse(warehouse_id, location)</p>
   <p>Inventory(product_id, warehouse_id , qty)</p>

<li>Problem 3: When the Api receives a POST request , data = request.json converts the incoming JSON payload into a key value dictionary.</li>
<p>When the code uses bracket Notation like data['name'], it is strictly demanding that the key "name" exist in that dictionary. However , the case study mentions that some fields might be optional.</p>
<p>If a client sends {"sku" : "123"  , "price" : 10.0} without a "name" field , data['name'] will immediately throw a KeyError and halt execution.</p>
<p>because the original code doesn't have a try/except block or any validation logic to catch that KeyError , the exception bubbles all the way up to the web framework.</p>
<li>500 Internal Server Error: The framework catches the unhandled exception assumes the backend code is broken, and returns 500 status. This Triggers server alarms ad makes it look like a backend bug.</li>
<li>400 Bad Request : A missing field isn't server ; its a client Mistake. The code should validate the data , reject the request gracefully and return a 400 status telling the client "hey your Json is missing the 'name' field."</li>
<li>Instead of data['name'] using data.get('name') is safer. If the key is missing, .get() safely return null value instead of crashing the application, allowing you to handle the missing data gracefully.</li>

# Part 2 : Database Design 

<p>Questions for Product Teams I would like to ask</p>
<ul>
    <li>Do we alllow the inventory quantity to drop below zero to handle backorders , or should the database strictly enforce quantity should be greater than or equal to zero ?</li>
    <li>Are Bundles pre-assemebled and stored as phusical units in the warehouse or are they assembled "on the fly" ?</li>
    <li>IF prices change depending on who is buying , we can't just alp a single price tag on the product in the databse. We have to design the database so that company A and company B each have their own private price book for the same supplier.</li>
</ul>

<p>My Design choice Justification</p>
<ul>
    <li>Append Only Ledger (inv_log) : In real ware houses things break get stolen or get lost. IF a warehouse manager asks , "why are we missing 50 laptops ? " , an apped only ledger lets you look back at every exact movement to investigate. It also makes calculating sales velocity incredibly easy.</li>
    <li>Many to Many : (Self Referencing)  We have one Products table . We create abundle link table where both the "Parent" column and the "child" column points right back to Product table. It allow for infinite nesting changing the database structure, A oriduct can be made of sub product , which are made of sub sub products.</li>
    <li>Indexing Strategy: Without an index , the app works fine with 100 products but will crash from timeouts when it hits 1000000 products . Building the index upfront proves you understand real-world scalability.</li>
</ul>

# Part 3 : API DESIGN 
<p>Assumptions and Missing Info Handled</p>
<p>To build this without complete requirements I made a few practical architectural assumptions</p>
<ul>
    <li>Database Heavy Lifting Our data access layer db get inv handles the complex SQL JOINs across the Product Warehouse Inventory and Supplier tables before the data hits this API</li>
    <li>Pre calculated Velocity Recent sales activity is handled asynchronously eg via a cron job and stored as a simple daily sales velocity integer vlc</li>
    <li>Dynamic Thresholds The low stock threshold is not a hardcoded magic number It dynamically adjusts based on the products type eg perishable vs durable</li>
</ul>

<p>Edge Cases Handled gracefully</p>
<p>Real world data is messy Here is how the code protects the server from crashing</p>
<ul>
    <li>Division by Zero If a product had a sudden spike in sales but historical velocity rounds to 0 calculating the days until stockout qty vlc would cause a fatal crash A ternary operator catches this and safely defaults to 999 days</li>
    <li>Missing Supplier Data Startups often have incomplete profiles If a supplier has not provided a contact email directly injecting a null value might break downstream frontend clients This is handled via an empty check to provide a safe NA fallback</li>
    <li>Input Validation A quick sanity check ensures c id 0 is rejected immediately preventing unnecessary and potentially expensive database queries for malformed requests</li>
</ul>

<p>Why this approach</p>
<ul>
    <li>Defense in Depth Even though the database query is supposed to filter out items without recent sales vlc 0 I still included a zero check in the C layer before the division Never trust the layer below you implicitly</li>
    <li>O N Time Complexity The code iterates through the returned dataset exactly once It applies the threshold business logic and formats the JSON payload in a single efficient pass</li>
    <li>Memory Efficiency By letting the databases WHERE clause filter out products with zero recent sales before handing the data to the API we prevent loading thousands of irrelevant inventory records into the applications RAM</li>
</ul>
</body>
</html>
