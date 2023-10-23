// index.js

document.addEventListener('DOMContentLoaded', function () {
    const ctx = document.getElementById('dataChart').getContext('2d');
    const labels = ['Nitrogen', 'Phosphorus', 'Potassium', 'pH', 'Temperature', 'Moisture'];

    // Function to fetch and update data
    function fetchDataAndUpdate() {
        fetch('get_data.php')
            .then(response => response.json())
            .then(data => {
                const values = [data.nitrogen, data.phosphorus, data.potassium, data.pH, data.temperature, data.moisture];

                // Check if the chart exists, if not, create it
                if (typeof window.myChart === 'undefined') {
                    window.myChart = new Chart(ctx, {
                        type: 'bar',
                        data: {
                            labels: labels,
                            datasets: [{
                                label: 'Data',
                                data: values,
                                backgroundColor: [
                                    'rgba(255, 99, 132, 0.2)',
                                    'rgba(54, 162, 235, 0.2)',
                                    'rgba(255, 206, 86, 0.2)',
                                    'rgba(75, 192, 192, 0.2)',
                                    'rgba(153, 102, 255, 0.2)'
                                ],
                                borderColor: [
                                    'rgba(255, 99, 132, 1)',
                                    'rgba(54, 162, 235, 1)',
                                    'rgba(255, 206, 86, 1)',
                                    'rgba(75, 192, 192, 1)',
                                    'rgba(153, 102, 255, 1)'
                                ],
                                borderWidth: 1
                            }]
                        },
                        options: {
                            scales: {
                                y: {
                                    beginAtZero: true
                                }
                            }
                        }
                    });
                } else {
                    // Update existing chart data
                    window.myChart.data.datasets[0].data = values;
                    window.myChart.update();
                }
            })
            .catch(error => {
                console.error('Error fetching data:', error);
            });
    }

    // Fetch and update data initially
    fetchDataAndUpdate();

    // Set interval to fetch and update data every 1 second
    setInterval(fetchDataAndUpdate, 1000);
});
