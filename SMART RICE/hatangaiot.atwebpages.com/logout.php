<?php
// Destroy the session to log out the user
session_start();
session_destroy();
header('Location: login.html');
?>
