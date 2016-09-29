var socket = io.connect(location.host);

// initial subscription
socket.emit("subscribe", "C");

var ctx = document.getElementById("graph").getContext("2d");

var myChart;

var datasets = [];

var options = {
    responsive: true,
    scales: {
        yAxes: [ {
            ticks: {}
        } ]
    }
};

socket.on("connect", function() {
    // handle received payloads
    socket.on("load", function(msg) {
        var idx;
        var label = "L" + msg.phase;
        var index = -1;
        for (idx = 0; idx < datasets.length; idx++) {
            if (datasets[idx].label === label) index = idx;
        }
        if (index === -1) {
            var red = Math.floor(Math.random() * 255);
            var green = Math.floor(Math.random() * 255);
            var blue = Math.floor(Math.random() * 255);
            dataset = {
                label: label,
                fill: false,
                borderColor: "rgba(" + red + "," + green + "," + blue + ",1)",
                data: msg.data
            };
            datasets.push(dataset);
        } else {
            datasets[index].data = msg.data;
        }
        drawChart();
    });
});

function drawChart() {
    if (myChart === undefined) {
        var data;
        var labels = [];
        for (var i = 0; i < 32; i++) labels.push(i);
        data = {
            labels: labels,
            datasets: datasets
        };
        myChart = new Chart(ctx, {
            type: "line",
            data: data,
            options: options
        });
    } else {
        myChart.data.datasets = datasets;
        myChart.update();
    }
}

function handleSel(opt) {
    socket.emit("subscribe", opt.value);
    if (myChart !== undefined) {
        datasets = [];
        myChart.destroy();
    }
}
