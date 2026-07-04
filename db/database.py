import sqlite3

DB_NAME = "database.db"

def get_db_connection():
    conn = sqlite3.connect(DB_NAME)
    conn.row_factory = sqlite3.Row
    return conn

def init_db():
    conn = get_db_connection()
    
    conn.execute("""
        CREATE TABLE IF NOT EXISTS remotes(
            id INTEGER PRIMARY KEY AUTOINCREMENT,
            label TEXT NOT NULL
        )
    """)

    conn.execute("""
        CREATE TABLE IF NOT EXISTS buttons(
            id INTEGER PRIMARY KEY AUTOINCREMENT,
            remote_id INTEGER,
            label TEXT,
            message TEXT
        )
    """)

    conn.commit()
    conn.close()
    return


# remote interface functions
def create_remote(label):
    conn = get_db_connection()

    conn.execute("INSERT INTO remotes (label) VALUES (?)", (label,) ) 

    conn.commit()
    conn.close()
    return

def get_remote(remote_id):
    conn = get_db_connection()

    row = conn.execute("SELECT * FROM remotes WHERE id = ?", (remote_id,) ).fetchone() 
    
    conn.close()

    if row is None:
        return None
    
    return dict(row)

def get_all_remotes():
    conn = get_db_connection()

    rows = conn.execute("SELECT * FROM remotes").fetchall() 

    conn.close()

    if rows is None:
        return None
    
    return [dict(row) for row in rows]


def update_label_remote(remote_id, label):
    conn = get_db_connection()

    row = conn.execute(
        "SELECT 1 FROM remotes WHERE id = ?",
        (remote_id,)
    ).fetchone()

    if row is None:
        conn.close()
        raise ValueError(f"No remote found with id {remote_id}")

    conn.execute("UPDATE remotes SET label = ? WHERE id = ?", (label, remote_id,) ) 
    
    conn.commit()
    conn.close()
    return

def delete_remote(remote_id):
    conn = get_db_connection()

    row = conn.execute(
        "SELECT 1 FROM remotes WHERE id = ?",
        (remote_id,)
    ).fetchone()

    if row is None:
        conn.close()
        raise ValueError(f"No remote found with id {remote_id}")

    conn.execute("DELETE FROM buttons WHERE remote_id = ?", (remote_id,) ) 
    conn.execute("DELETE FROM remotes WHERE id = ?", (remote_id,) ) 

    conn.commit()
    conn.close()
    return
    
# button interface functions
def create_button(remote_id, label):
    conn = get_db_connection()

    row = conn.execute(
        "SELECT 1 FROM remotes WHERE id = ?",
        (remote_id,)
    ).fetchone()

    if row is None:
        conn.close()
        raise ValueError(f"No remote found with id {remote_id}")

    conn.execute("INSERT INTO buttons (remote_id, label) VALUES (?, ?)", (remote_id, label,) )

    conn.commit()
    conn.close()
    return

def get_buttons(remote_id):
    conn = get_db_connection()

    rows = conn.execute("SELECT * FROM buttons WHERE remote_id = ?", (remote_id,) ).fetchall() 

    conn.close()

    if not rows:
        return None
    
    return [dict(row) for row in rows]

def get_message_button(remote_id):
    conn = get_db_connection()

    row = conn.execute("SELECT message FROM buttons WHERE remote_id = ?", (remote_id,) ).fetchone() 

    conn.close()
    
    if row is None:
        return None
    
    return row["message"]


def update_message_button(button_id, message):
    conn = get_db_connection()

    conn.execute("UPDATE buttons SET message = ? WHERE id = ?", (message, button_id,) ) 
    
    conn.commit()
    conn.close()
    return

def update_label_button(remote_id, button_id, label):
    conn = get_db_connection()

    row = conn.execute(
        "SELECT 1 FROM buttons WHERE id = ? AND remote_id = ?",
        (button_id, remote_id,)
    ).fetchone()

    if row is None:
        conn.close()
        raise ValueError(f"No button found with id {button_id} and remote_id {remote_id}")

    conn.execute("UPDATE buttons SET label = ? WHERE id = ? AND remote_id = ?", (label, button_id, remote_id,) ) 

    conn.commit()
    conn.close()
    return

def delete_button(remote_id, button_id):
    conn = get_db_connection()

    row = conn.execute(
        "SELECT 1 FROM buttons WHERE id = ? AND remote_id = ?",
        (button_id, remote_id,)
    ).fetchone()

    if row is None:
        conn.close()
        raise ValueError(f"No button found with id {button_id} and remote_id {remote_id}")

    conn.execute("DELETE FROM buttons WHERE id = ? AND remote_id = ?", (button_id, remote_id,) ) 

    conn.commit()
    conn.close()
    return
