/*
 * -----------------------
 * RESTfulness in C.......
 * ***********************
 * Oskar Bahner Hansen....
 * cph-oh82@cphbusiness.dk
 * 2024-10-31.............
 * -----------------------
 */

const main_content = document.querySelector("#main_content");
const display_clock = document.querySelector("#display_clock");
const search_form = document.querySelector("#search_form");

search_form.addEventListener("submit", sendData);

function startTime() {
    const today = new Date();
    let h = today.getHours();
    let m = today.getMinutes();
    let s = today.getSeconds();
    m = checkTime(m);
    s = checkTime(s);
    display_clock.innerHTML =  h + ":" + m + ":" + s;
    setTimeout(startTime, 1000);
}

function checkTime(i) {
    if (i < 10) {i = "0" + i};  // add zero in front of numbers < 10
    return i;
}

async function sendData() {
    // Associate the FormData object with the form element
    const formData = new FormData(search_form);
    const city = formData.get("id")

    if (city.length < 2) {
        return;
    }

    try {
        //const weather_res = new XMLHttpRequest();
        //weather_res.open("GET", "/api/weather?city=" + city, false); // `false` makes the request synchronous
        //weather_res.send(null);
        
        //let weather_res = fetch("/api/weather?city=" + city);
        //return weather_res.json();
        
        fetch("/api/weather?city=" + city)
            .then(response => {
                if (!response.ok) {
                    throw new Error("HTTP error " + response.status);
                }
                return response.json();
            })
            .then(json => {
                update_main_content(json);
            });

        //fetch("/api/weather?city=" + city, {
        //})
        //    .then(response => {
        //        if (!response.ok) {
        //            throw new Error("HTTP error " + response.status);
        //        }
        //        return response.json();
        //    })
        //    .then(json => {
        //        let wr = document.createElement("div");
        //        wr.innerHTML = json.location.name;
        //        main_content.appendChild(wr);
        //    });
    }

    catch (e) {
        console.error(e);
        return null;
    }
}

// Take over form submission
search_form.addEventListener("submit", (event) => {
    event.preventDefault();
    sendData();
});

function update_main_content(json)
{
    //let new_div = document.createElement("div");
    //new_div.innerHTML = json.location.name;
    //main_content.appendChild(new_div);
    
    const table_elm = document.createElement("table");
    
    table_elm.style.width = '100px';
    table_elm.style.border = '1px solid black';
    table_elm.className = "weather_report"

    const weather_info = json.current;

    const th_name = table_elm.createTHead();
    const tr_name = th_name.insertRow();
    const td_name_0 = tr_name.insertCell();
    const td_name_1 = tr_name.insertCell();
    td_name_0.appendChild(document.createTextNode(`City`));
    td_name_1.appendChild(document.createTextNode(`${json.location.name}`));

    for (let key in weather_info) {
        let value = weather_info[key];

        if (typeof(value) == "object") {
            let str = "";
            for (let key in value) {
                str += `${key}: ${value[key]}\r\n`;
            }
            value = str;
        }
        
        const tr = table_elm.insertRow();
        const td_0 = tr.insertCell();
        const td_1 = tr.insertCell();
        td_0.appendChild(document.createTextNode(`${key}`));
        td_1.appendChild(document.createTextNode(`${value}`));
    }

    main_content.innerHTML = "";
    main_content.appendChild(table_elm);
}




















