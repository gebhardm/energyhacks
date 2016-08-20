var socket = io.connect(location.host);

var ctx = document.getElementById("graph").getContext("2d");

var myChart;

var options = {
    responsive: true
};

socket.on("connect", function() {
    socket.on("mqtt", function(msg) {
        var topic, mains, series;
        topic = msg.topic.split("/");
        mains = topic[5];
        series = msg.message[1];
        if (myChart === undefined) {
            data = {
                labels: [ 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15, 16, 17, 18, 19, 20, 21, 22, 23, 24, 25, 26, 27, 28, 29, 30, 31, 32 ],
                datasets: [ {
                    label: "L1",
                    fill: false,
                    borderColor: "#f00",
                    data: series
                }, {
                    label: "L2",
                    fill: false,
                    borderColor: "#0f0",
                    data: series
                }, {
                    label: "L3",
                    fill: false,
                    borderColor: "#00f",
                    data: series
                } ]
            };
            myChart = new Chart(ctx, {
                type: "line",
                data: data,
                options: options
            });
        } else {
            myChart.data.datasets[mains].data = series;
            myChart.update();
        }
    });
});