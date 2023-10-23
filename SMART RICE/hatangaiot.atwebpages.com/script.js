async function getHistogramData() {
  const response = await fetch("get_data.php");
  const data = await response.json();

  return data;
}

async function drawHistogram() {
  const data = await getHistogramData();

  // Get the maximum value of the data
  const maxValue = Math.max(...data);

  // Create a new histogram chart
  const chart = new Chart(".histogram", {
    type: "bar",
    data: {
      labels: ["Nitrogen", "Phosphorus", "Potassium", "pH", "Temperature"],
      datasets: [
        {
          label: "Data",
          data: data,
          backgroundColor: "#777",
          borderColor: "#000",
          borderWidth: 1,
        },
      ],
    },
    options: {
      scales: {
        yAxes: [
          {
            ticks: {
              max: maxValue,
              stepSize: maxValue / 10,
            },
          },
        ],
      },
      legend: {
        display: false,
      },
      tooltips: {
        enabled: true,
        callbacks: {
          label: function(tooltipItem) {
            return `${tooltipItem.xLabel}: ${tooltipItem.yLabel}`;
          },
        },
      },
    },
  });
}

// Draw the histogram when the page loads
window.addEventListener("load", drawHistogram);
