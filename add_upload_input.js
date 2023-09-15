var input = document.createElement("input");
input.type = "file";
document.body.appendChild(input);
input.onclick = () => {
  input.value = null;
};
input.onchange = () => {
  var fd = new FormData();
  fd.append("torrent", input.files[0]);

  fetch("/get_magnet", { method: "POST", body: fd })
    .then((e) => e.json())
    .then((e) => console.log(e))
    .catch((e) => console.error(e));
};
