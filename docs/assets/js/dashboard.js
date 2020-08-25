---
---

const chartjs = window.Chart.min;

window.onload = function() {
    generateGraphs();
};

function generateGraphs() {
    // @ts-ignore
    var data = {{ site.data.charts | jsonify }};
    data = data["AverageFPS"]
    console.log(data);

    var ctx = document.getElementById('avgFPSChart').getContext('2d');
    new Chart(ctx, {
        type: 'line',
        data: {
            labels: data['labels'],
            datasets: [
                {
                    data: data['vulkan'],
                    label: 'Vulkan',
                    backgroundColor: 'rgba(255, 99, 132, 0.2)',
                    borderColor: 'rgba(255, 99, 132, 1)',
                    borderWidth: 1
                },
                {
                    data: data["directx11"],
                    label: "DirectX 11",
                    backgroundColor: 'rgba(54, 162, 235, 0.2)',
                    borderColor: 'rgba(54, 162, 235, 1)',
                    borderWidth: 1
                }
            ]
        },
        options: {
            title: {
                display: true,
                text: 'Average FPS'
            },
            responsive: true,
            scales: {
                xAxes: [{
                    ticks: {
                        autoSkip: false,
                        maxRotation: 77,
                        minRotation: 77
                    }
                }],
                yAxes: [{
                    ticks: {
                        beginAtZero: true
                    }
                }]
            },
            tooltips: {
                mode: 'index',
                callbacks: {
                    afterTitle: function(tooltipItem, _) {
                        return data['tooltips'][tooltipItem[0].index];
                    }
                }
            }
        }
    });
}

function loadFile(filePath) {
    var result = null;
    var xmlhttp = new XMLHttpRequest();
    xmlhttp.open("GET", filePath, false);
    xmlhttp.send();
    if (xmlhttp.status==200) {
      result = xmlhttp.responseText;
    }
    return result;
  }
