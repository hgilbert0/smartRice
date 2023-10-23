<?php
// Database connection
try {
    $db = new PDO('mysql:host=fdb1033.awardspace.net;dbname=4376220_readings', '4376220_readings', 'Hategeka@2023');
    $db->setAttribute(PDO::ATTR_ERRMODE, PDO::ERRMODE_EXCEPTION);

    // Execute an SQL query to retrieve the latest data from your database
    $query = $db->query("SELECT nitrogen, phosphorus, potassium, pH, temperature, moisture FROM readings ORDER BY time_of_insertion DESC LIMIT 1");
    $data = $query->fetch(PDO::FETCH_ASSOC);

    // Return the data as JSON
    header("Content-Type: application/json");
    echo json_encode($data);
} catch (PDOException $e) {
    // Handle any database-related errors here
    header("HTTP/1.1 500 Internal Server Error");
    echo json_encode(["error" => "Database error: " . $e->getMessage()]);
}
?>
