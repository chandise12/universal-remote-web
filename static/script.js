
const emptyRemoteError = document.getElementById("error-empty-remote");

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
                emptyRemoteError.textContent = "Please enter a name for this remote!";
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

}