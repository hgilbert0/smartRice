<?php
if ($_SERVER['REQUEST_METHOD'] === 'POST') {
    // Replace these with your database connection details
    $db_host = 'fdb1033.awardspace.net';
    $db_name = '4376220_readings';
    $db_user = '4376220_readings';
    $db_pass = 'Hategeka@2023';

    $username = $_POST['username'];
    $password = $_POST['password'];

    try {
        $conn = new PDO("mysql:host=$db_host;dbname=$db_name", $db_user, $db_pass);
        $conn->setAttribute(PDO::ATTR_ERRMODE, PDO::ERRMODE_EXCEPTION);

        $stmt = $conn->prepare("SELECT * FROM users WHERE username = :username");
        $stmt->bindParam(':username', $username);
        $stmt->execute();
        $row = $stmt->fetch();

        if ($row && password_verify($password, $row['password'])) {
            // Both username and password are correct
            header("Location: index.html");
            exit();
        } else {
            // Incorrect username or password
            header("Location: login.html?error=auth");
            exit();
        }
    } catch (PDOException $e) {
        echo "Error: " . $e->getMessage();
    }
}
