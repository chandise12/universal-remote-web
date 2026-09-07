
const emptyCreateRemoteError = document.getElementById("error-empty-remote");

const remoteForm = document.getElementById("create-remote");
if(remoteForm){
    remoteForm.addEventListener('submit', async(event) => {
        event.preventDefault();

        const formData = new FormData(remoteForm);
        const dataObject = Object.fromEntries(formData.entries());

        try{
            const response = await fetch('/remotes',{
                method: 'POST',
                body: JSON.stringify(dataObject),
                headers: {
                    'Content-Type': 'application/json'
                }
            });
            const result = await response.json();

            if (response.ok) {
                window.location.reload();
            } else {
                console.error("Server error:", result);
                emptyCreateRemoteError.textContent = "Please enter a name for this remote!";
            }

            console.log('Success:', result);

        }catch(error) {
            console.error('Error:', error);
        }   
    });


    const deleteRemote = document.querySelectorAll(".delete-remote");

    deleteRemote.forEach(button => {
        button.addEventListener('click', async() => {

            const id = button.dataset.remoteId;

            try{
                const response = await fetch(`/remotes/${id}`,{
                    method: 'DELETE'
                });

                if (response.ok) {
                    window.location.reload();
                }
                
            }catch(error) {
                console.error('Error:', error);
            }   
        });
    });


    const renameRemote = document.querySelectorAll(".rename-remote");

    renameRemote.forEach(button =>{

        button.addEventListener("click", () => {
            const listItem = button.closest("li");
            const form = listItem.querySelector(".rename-remote-form");

            button.hidden = true;
            form.hidden = false;
        });
    });


    const cancelRenameRemote = document.querySelectorAll(".cancel-rename-remote");

    cancelRenameRemote.forEach(button => {

        button.addEventListener("click", () => {

            const listItem = button.closest("li");
            const form = listItem.querySelector(".rename-remote-form");
            const renameButton = listItem.querySelector(".rename-remote");
            const error = form.querySelector(".error-empty-remote");

            form.hidden = true;
            renameButton.hidden = false;
            error.textContent = "";
        });
    });


    const renameRemoteForm = document.querySelectorAll(".rename-remote-form");

    renameRemoteForm.forEach(form => {

        form.addEventListener("submit", async (event) => {

            event.preventDefault();

            const remoteId = form.dataset.remoteId;

            const formData = new FormData(form);
            const data = Object.fromEntries(formData.entries());

            const emptyRenameRemoteError = form.querySelector(".error-empty-remote");

            try {

                const response = await fetch(`/remotes/${remoteId}`, {
                    method: "PUT",
                    headers: {
                        "Content-Type": "application/json"
                    },
                    body: JSON.stringify(data)
                });

                if (response.ok) {
                    window.location.reload();
                }else{
                    emptyRenameRemoteError.textContent = "Please enter a name for this remote!";
                }

            } catch (error) {
                console.error(error);
            }

        });

    });


}





const emptyButtonError = document.getElementById("error-empty-button");

const buttonForm = document.getElementById("create-button");

if(buttonForm){
    buttonForm.addEventListener('submit', async(event) => {
        event.preventDefault();

        const formData = new FormData(buttonForm);
        const dataObject = Object.fromEntries(formData.entries());

        const r_id = buttonForm.dataset.remoteId;

        try{
            const response = await fetch(`/remotes/${r_id}/buttons`,{
                method: 'POST',
                body: JSON.stringify(dataObject),
                headers: {
                    'Content-Type': 'application/json'
                }
            });
            const result = await response.json();

            if (response.ok) {
                window.location.reload();
            } else {
                console.error("Server error:", result);
                emptyButtonError.textContent = "Please enter a name for this button!";
            }

            console.log('Success:', result);

        }catch(error) {
            console.error('Error:', error);
        }   
    });


    const deleteButton = document.querySelectorAll(".delete-button");

    deleteButton.forEach(button => {
        button.addEventListener('click', async() => {

            const r_id = button.dataset.remoteId;
            const b_id = button.dataset.buttonId;

            try{
                const response = await fetch(`/remotes/${r_id}/buttons/${b_id}`,{
                    method: 'DELETE'
                });

                if (response.ok) {
                    window.location.reload();
                }
                
            }catch(error) {
                console.error('Error:', error);
            }   
        });
    });


    const commandButtons = document.querySelectorAll(".command-button");

    commandButtons.forEach(button => {
        button.addEventListener("click", async () => {

            const remoteId = button.dataset.remoteId;
            const buttonId = button.dataset.buttonId;

            try {
                const response = await fetch(
                    `/remotes/${remoteId}/buttons/${buttonId}/command`,
                    {
                        method: "POST"
                    }
                );

                const result = await response.json();

                if (response.ok) {
                    console.log(result.status);
                } else {
                    console.error(result);
                }

            } catch (error) {
                console.error(error);
            }
        });
    });




    const renameButton = document.querySelectorAll(".rename-button");

    renameButton.forEach(button =>{

        button.addEventListener("click", () => {
            const listItem = button.closest("li");
            const form = listItem.querySelector(".rename-button-form");

            button.hidden = true;
            form.hidden = false;
        });
    });


    const cancelRenameButton = document.querySelectorAll(".cancel-rename-button");

    cancelRenameButton.forEach(button => {

        button.addEventListener("click", () => {

            const listItem = button.closest("li");
            const form = listItem.querySelector(".rename-button-form");
            const renameButton = listItem.querySelector(".rename-button");
            const error = form.querySelector(".error-empty-button");

            form.hidden = true;
            renameButton.hidden = false;
            error.textContent = "";
        });
    });


    const renameButtonForm = document.querySelectorAll(".rename-button-form");

    renameButtonForm.forEach(form => {

        form.addEventListener("submit", async (event) => {

            event.preventDefault();

            const remoteId = form.dataset.remoteId;
            const buttonId = form.dataset.buttonId;

            const formData = new FormData(form);
            const data = Object.fromEntries(formData.entries());

            const emptyRenameRemoteError = form.querySelector(".error-empty-button");

            try {

                const response = await fetch(`/remotes/${remoteId}/buttons/${buttonId}`, {
                    method: "PUT",
                    headers: {
                        "Content-Type": "application/json"
                    },
                    body: JSON.stringify(data)
                });

                if (response.ok) {
                    window.location.reload();
                }else{
                    emptyRenameRemoteError.textContent = "Please enter a name for this remote!";
                }

            } catch (error) {
                console.error(error);
            }

        });

    });

}