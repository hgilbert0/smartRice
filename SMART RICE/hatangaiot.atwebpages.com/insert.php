<?php
// Check if the POST request contains a valid key (authentication)

$key = "";
if (isset($_POST['key'])) {
  $key = $_POST['key'];
}

if ($key != "a@4K%3") {
  error_log("Unauthorized: Key mismatch - Received Key: $key");
  echo "Unauthorized";
  exit();
}

// Check if the POST request contains CSV data in the request body
$requestData = file_get_contents("php://input");

if (empty($requestData)) {
  error_log("CSV data not found in the request body");
  echo "CSV data not found";
  exit();
}

// CSV data is present in the request body, proceed to process it
// Split and process the CSV data
$csvData = $requestData;
$dataArray = explode(",", $csvData);

// Remove the `key=a@4K%3&post_data=` part from the first element of the array
$regex = "/^key=a@4K%3&post_data=/";
$dataArray[0] = preg_replace($regex, "", $dataArray[0]);

if (count($dataArray) != 6) {
  error_log("Invalid CSV format");
  echo "Invalid CSV format";
  exit();
}
//echo "Received data: " . $csvData;

// Extract data
list($nitrogen, $phosphorus, $potassium, $pH, $temperature, $moisture) = $dataArray;

// Get the server's time (local time + 2 hours)
$time_of_insertion = date('Y-m-d H:i:s', strtotime('+2 hours'));

// Database connection
try {
  $db = new PDO('mysql:host=fdb1033.awardspace.net;dbname=4376220_readings', '4376220_readings', 'Hategeka@2023');
  $db->setAttribute(PDO::ATTR_ERRMODE, PDO::ERRMODE_EXCEPTION);
   
  // Prepare and execute the SQL INSERT query
  $query = $db->prepare("INSERT INTO readings (nitrogen, phosphorus, potassium, pH, temperature, moisture, time_of_insertion) VALUES (:nitrogen, :phosphorus, :potassium, :pH, :temperature, :moisture, :time_of_insertion)");
  // Bind the nitrogen variable to the :nitrogen placeholder
  $query->bindParam(':nitrogen', $nitrogen, PDO::PARAM_STR);
  // Bind the other variables to the other placeholders
  $query->bindParam(':phosphorus', $phosphorus, PDO::PARAM_STR);
  $query->bindParam(':potassium', $potassium, PDO::PARAM_STR);
  $query->bindParam(':pH', $pH, PDO::PARAM_STR);
  $query->bindParam(':temperature', $temperature, PDO::PARAM_STR);
  $query->bindParam(':moisture', $moisture, PDO::PARAM_STR);
  $query->bindParam(':time_of_insertion', $time_of_insertion);
  $query->execute();
   
  echo "Data inserted successfully";
} catch (PDOException $e) {
  error_log("Database error: " . $e->getMessage());
  echo "Database error: " . $e->getMessage();
}
?>