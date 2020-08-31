$(document).ready(function()
{
    // OBTENEMOS LA CARD DE CADA SERVIDOR
    let servidor_card1 = document.getElementById("servidor_card1");
    let servidor_card2 = document.getElementById("servidor_card2");
    let servidor_card3 = document.getElementById("servidor_card3");
    // OBTENEMOS EL DIV DONDE PONDREMOS EL CONTENIDO DE CADA CARD DE SERVIDOR
    let server_info_container_1 = document.getElementById("server_info_container_1");
    let server_info_container_2 = document.getElementById("server_info_container_2");
    let server_info_container_3 = document.getElementById("server_info_container_3");
    // OBTENEMOS EL BOTÓN DONDE MOSTRAMOS EL NÚMERO DE VOTOS TOTALES
    let votos_totales_btn = document.getElementById("votos_totales_btn");  
    // CONFIGURAMOS Y CREAMOS EL CHART
    var ctx = document.getElementById('myChart').getContext('2d');
    var chart = new Chart(ctx, 
    {
        type: 'bar',
        data: {
            labels: [0],
            datasets: [{
                label: 'Votos en el tiempo',
                backgroundColor: "#90caf9",
                borderColor: "#90caf9",
                data: [0]
            }],
        },
        options:
        {
            scales: 
            {
                yAxes: 
                [{
                    scaleLabel: 
                    {
                        display: true,
                        labelString: 'Votos'
                    }
                }],
                xAxes: 
                [{
                    scaleLabel: 
                    {
                        display: true,
                        labelString: 'Min:Seg'
                    }
                }]
            }     
        }
    });
    // FUNCIÓN PARA AÑADIR NUEVOS DATOS AL CHART
    function addData(chart, label, data) 
    {
        chart.data.labels.push(label);
        chart.data.datasets.forEach((dataset) => {
            dataset.data.push(data);
        });
        chart.update();
    }
    // MANEJAMOS UNA VAR PARA ALMACENAR EL NÚMERO DE VOTOS REGISTRADOS POR CADA SERVIDOR Y OTRA PARA EL NÚMERO TOTAL DE VOTOS
    let server_votos_1 = 0;
    let server_votos_2 = 0;
    let server_votos_3 = 0;
    let total_num_votos = 0;
    // OBTENEMOS EL TIEMPO EN MILISEGUNDOS AL INICIAR LA APLICACIÓN
    let start_time = Date.now();
    // TIMER QUE LLAMA Al ENDPOINT RESTFUL /statistics EN EL SERVIDOR WEB QUIEN NOS REGRESARÁ EL NÚMERO DE VOTOS Y LA IP DE CADA SERVIDOR
    setInterval(function()
    {
        // USANDO AJAX ENVIAMOS LA PETICIÓN REST 
        $.ajax(
        {
            type: "POST",               // UTILIZANDO EL MÉTODO HTTP POST
            url: "/statistics",         // ENDPOINT RESTFUL 
            dataType: "json",           // INDICANDO QUE ESPERAMOS RECIBIR DE RESPUESTA UN OBJETO JSON
            success: function(respAx)   // FUNCIÓN QUE SE EJECUTA AL SER EXITOSA LA OPERACIÓN
            {
                // console.log(respAx);
                // console.log(respAx.servidor1);

                total_num_votos = 0;
                // CUANDO EL NÚMERO DE VOTOS DE UN SERVIDOR ES IGUAL A "-1" ESTE SE ENCUENTRA OFFLINE
                if(respAx.servidor1.num_votos != -1)
                {
                    server_info_container_1.innerHTML = "";
                    servidor_card1.classList.remove("red");
                    servidor_card1.classList.remove("lighten-1");
    
                    servidor_card1.classList.add("teal");
                    servidor_card1.classList.add("lighten-2");

                    let server_tag = document.createElement("h6");
                    let new_server_text = document.createTextNode("ONLINE");
                    server_tag.appendChild(new_server_text);
                    let server_ip = document.createElement("h6");
                    new_server_text = document.createTextNode(respAx.servidor1.ip);
                    server_ip.appendChild(new_server_text);
                    let server_votos = document.createElement("h6");
                    new_server_text = document.createTextNode("Votos: " + respAx.servidor1.num_votos);
                    server_votos.appendChild(new_server_text);
                    
                    server_info_container_1.appendChild(server_tag);
                    server_info_container_1.appendChild(server_ip);
                    server_info_container_1.appendChild(server_votos);

                    server_votos_1 = respAx.servidor1.num_votos;
                }
                else
                {
                    server_info_container_1.innerHTML = "";
                    servidor_card1.classList.remove("teal");
                    servidor_card1.classList.remove("lighten-2");
    
                    servidor_card1.classList.add("red");
                    servidor_card1.classList.add("lighten-1");

                    let server_tag = document.createElement("h6");
                    let new_server_text = document.createTextNode("OFFLINE");
                    server_tag.appendChild(new_server_text);

                    server_info_container_1.appendChild(server_tag);
                }

                if(respAx.servidor2.num_votos != -1)
                {
                    server_info_container_2.innerHTML = "";
                    servidor_card2.classList.remove("red");
                    servidor_card2.classList.remove("lighten-1");
    
                    servidor_card2.classList.add("teal");
                    servidor_card2.classList.add("lighten-2");

                    let server_tag = document.createElement("h6");
                    let new_server_text = document.createTextNode("ONLINE");
                    server_tag.appendChild(new_server_text);
                    let server_ip = document.createElement("h6");
                    new_server_text = document.createTextNode(respAx.servidor2.ip);
                    server_ip.appendChild(new_server_text);
                    let server_votos = document.createElement("h6");
                    new_server_text = document.createTextNode("Votos: " + respAx.servidor2.num_votos);
                    server_votos.appendChild(new_server_text);
                    
                    server_info_container_2.appendChild(server_tag);
                    server_info_container_2.appendChild(server_ip);
                    server_info_container_2.appendChild(server_votos);

                    server_votos_2 = respAx.servidor2.num_votos;
                }
                else
                {
                    server_info_container_2.innerHTML = "";
                    servidor_card2.classList.remove("grey");
                    servidor_card2.classList.remove("teal");
                    servidor_card2.classList.remove("lighten-2");
    
                    servidor_card2.classList.add("red");
                    servidor_card2.classList.add("lighten-1");

                    let server_tag = document.createElement("h6");
                    let new_server_text = document.createTextNode("OFFLINE");
                    server_tag.appendChild(new_server_text);

                    server_info_container_2.appendChild(server_tag);
                }

                if(respAx.servidor3.num_votos != -1)
                {
                    server_info_container_3.innerHTML = "";
                    servidor_card3.classList.remove("grey");
                    servidor_card3.classList.remove("red");
                    servidor_card3.classList.remove("lighten-1");
    
                    servidor_card3.classList.add("teal");
                    servidor_card3.classList.add("lighten-2");

                    let server_tag = document.createElement("h6");
                    let server_text = document.createTextNode("ONLINE");
                    server_tag.appendChild(server_text);
                    let server_ip = document.createElement("h6");
                    new_server_text = document.createTextNode(respAx.servidor3.ip);
                    server_ip.appendChild(new_server_text);
                    let server_votos = document.createElement("h6");
                    new_server_text = document.createTextNode("Votos: " + respAx.servidor3.num_votos);
                    server_votos.appendChild(new_server_text);
                    
                    server_info_container_3.appendChild(server_tag);
                    server_info_container_3.appendChild(server_ip);
                    server_info_container_3.appendChild(server_votos);

                    server_votos_3 = respAx.servidor3.num_votos;
                }
                else
                {
                    server_info_container_3.innerHTML = "";
                    servidor_card3.classList.remove("grey");
                    servidor_card3.classList.remove("teal");
                    servidor_card3.classList.remove("lighten-2");
    
                    servidor_card3.classList.add("red");
                    servidor_card3.classList.add("lighten-1");

                    let server_tag = document.createElement("h6");
                    let new_server_text = document.createTextNode("OFFLINE");
                    server_tag.appendChild(new_server_text);

                    server_info_container_3.appendChild(server_tag);
                }

                // console.log(total_num_votos);
                // ACTUALIZAMOS EL BOTÓN DONDE SE MUESTRAN LOS VOTOS TOTALES
                total_num_votos = server_votos_1 + server_votos_2 + server_votos_3;
                votos_totales_btn.innerHTML = "Votos totales: " + total_num_votos;
                // OBTENEMOS EL MOMENTO EN MILISEGUNDOS EN QUE SE TERMINA DE RECIBIR LOS DATOS
                let stop_time = Date.now();
                // CALCULAMOS EL TIEMPO QUE HA PASADO DESDE QUE SE INICIÓ LA EJECUCIÓN DE LA APLICACIÓN Y LO CONVERTIMOS A UN OBJETO DATE
                // PARA PODER USAR SUS MÉTODOS COMO GETTIME()
                let new_date = new Date(stop_time - start_time);
                // console.log(new_date.getMinutes());
                // console.log(new_date.getSeconds());
                // AÑADIMOS EL NUEVO PAR DE DATOS AL CHART, ESTO ES EL TIEMPO TRANSCURRIDO Y EL NÚMERO DE VOTOS CORRESPONDIENTES
                addData(chart, new_date.getMinutes() + " : " + new_date.getSeconds(), total_num_votos);
            },
            error: function(xhr, ajaxOptions, thrownError)
            {
                // ------------------------ SERVER 1 --------------------------
                server_info_container_1.innerHTML = "";
                servidor_card1.classList.remove("grey");
                servidor_card1.classList.remove("teal");
                servidor_card1.classList.remove("lighten-2");
                servidor_card1.classList.add("red");
                servidor_card1.classList.add("lighten-1");
                let server_tag1 = document.createElement("h6");
                let new_server_text1 = document.createTextNode("OFFLINE");
                server_tag1.appendChild(new_server_text1);
                server_info_container_1.appendChild(server_tag1);

                // ------------------------ SERVER 2 --------------------------
                server_info_container_2.innerHTML = "";
                servidor_card2.classList.remove("grey");
                servidor_card2.classList.remove("teal");
                servidor_card2.classList.remove("lighten-2");
                servidor_card2.classList.add("red");
                servidor_card2.classList.add("lighten-1");
                let server_tag2 = document.createElement("h6");
                let new_server_text2 = document.createTextNode("OFFLINE");
                server_tag2.appendChild(new_server_text2);
                server_info_container_2.appendChild(server_tag2);

                // ------------------------ SERVER 3 --------------------------
                server_info_container_3.innerHTML = "";
                servidor_card3.classList.remove("grey");
                servidor_card3.classList.remove("teal");
                servidor_card3.classList.remove("lighten-2");
                servidor_card3.classList.add("red");
                servidor_card3.classList.add("lighten-1");
                let server_tag3 = document.createElement("h6");
                let new_server_text3 = document.createTextNode("OFFLINE");
                server_tag3.appendChild(new_server_text3);
                server_info_container_3.appendChild(server_tag3);
            }
        });
    }, 500);
});