# Bynry-Inc
Case Study
# Part 1 : Code Review and Debugging
<h3>Problem 1:</h3> The code exceutes two separate commits. If the Inventory creation fails(for example database timeout), the Product is already saved. In production , this creates orphaned products that cannot be properly tracked or sold. <br><br>
<h3>Problem 2:</h3> When you put warehouse_id directly inside the Product table(or pass it to its consturctor) , you are dedicating a single column to store where that products lives. Because a single cell ina adatabse row can only hold one value , a product like "ABC" can only be assigned to "warehouse_1". This create one to many relationship: one warehouse can hold many products but each product is permanently locked to exactly one warehouse. <br>
However it is given in problem statement that: Products can exist in multiple warehouses. <br>
Products can be stored in multiple warehouses with different quantities. <br>
If "BC" needs to be in both wareshouse_1 and warehouse_2 the 1 to many design would force you to create the completely duplicate "ABC" row just to assign it a new warehouse_id. This leads to data duplication and tracking nightmares.
<br>The Solution is as follow : Many to Many , Product table should know anything about the warehouse.  
<br> Product(product_id, name) <br> Warehouse(warehouse_id, location) <br> Inventory(product_id, warehouse_id , qty) 
<h3>Problem 3:</h3> When the Api receives a POST request , data = request.json converts the incoming JSON payload into a key value dictionary. <br>
When the code uses bracket Notation like data['name'], it is strictly demanding that the key "name" exist in that dictionary. However , the case study mentions that some fields might be optional. <br> <br>
If a client sends {"sku" : "123"  , "price" : 10.0} without a "name" field , data['name'] will immediately throw a KeyError and halt execution. <br> <br>
because the original code doesn't have a try/except block or any validation logic to catch that KeyError , the exception bubbles all the way up to the web framework. <br> 
500 Internal Server Error: The framework catches the unhandled exception assumes the backend code is broken, and returns 500 status. This Triggers server alarms ad makes it look like a backend bug.<br>
400 Bad Request : A missing field isn't server ; its a client Mistake. The code should validate the data , reject the request gracefully and return a 400 status telling the client "hey your Json is missing the 'name' field." 
<br> <br>
Instead of data['name'] using data.get('name') is safer. If the key is missing, .get() safely return null value instead of crashing the application, allowing you to handle the missing data gracefully.
