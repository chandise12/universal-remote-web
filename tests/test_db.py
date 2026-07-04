from database import (
    init_db,
    create_remote,
    get_remote,
    get_all_remotes,
    update_label_remote,
    delete_remote,
    create_button,
    get_buttons,
    get_message_button,
    update_message_button,
    update_label_button,
    delete_button
)

def run():
    print("INIT DB")
    init_db()

    print("\nCREATE REMOTE")
    create_remote("TV Remote")

    remotes = get_all_remotes()
    print("REMOTES:", remotes)

    remote_id = remotes[0]["id"]
    print("REMOTE ID:", remote_id)

    print("\nGET REMOTE")
    print(get_remote(remote_id))

    print("\nUPDATE REMOTE LABEL")
    update_label_remote(remote_id, "Updated TV Remote")
    print(get_remote(remote_id))

    print("\nCREATE BUTTONS")
    create_button(remote_id, "Power")
    create_button(remote_id, "Volume Up")

    buttons = get_buttons(remote_id)
    print("BUTTONS:", buttons)

    button_id = buttons[0]["id"]
    print("BUTTON ID:", button_id)

    print("\nUPDATE BUTTON LABEL")
    update_label_button(button_id, "Power Updated")

    print(get_buttons(remote_id))

    print("\nUPDATE BUTTON MESSAGE")
    update_message_button(button_id, "IR_SIGNAL_123")

    print("MESSAGE:")
    print(get_message_button(remote_id))

    print("\nDELETE BUTTON")
    delete_button(button_id)
    print(get_buttons(remote_id))

    print("\nDELETE REMOTE")
    delete_remote(remote_id)
    print(get_all_remotes())

if __name__ == "__main__":
    run()