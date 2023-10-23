// login.js

document.addEventListener('DOMContentLoaded', function () {
    const form = document.querySelector('form');
    form.addEventListener('submit', function (e) {
        e.preventDefault();
        const username = document.getElementById('username').value;
        const password = document.getElementById('password').value;

        fetch('login.php', {
            method: 'POST',
            body: new URLSearchParams({ username, password }),
            headers: {
                'Content-Type': 'application/x-www-form-urlencoded'
            }
        })
        .then(response => {
            if (response.url.includes('error=auth')) {
                // Display an error message for incorrect login
                const errorMessage = document.querySelector('.error-message');
                errorMessage.textContent = 'Incorrect username or password';
            }
        })
        .catch(error => {
            console.error('Error:', error);
        });
    });
});
