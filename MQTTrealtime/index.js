var socket = io.connect(location.host);

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
        data = {
            labels: [ 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15, 16, 17, 18, 19, 20, 21, 22, 23, 24, 25, 26, 27, 28, 29, 30, 31, 32 ],
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