import tkinter as tk
from tkinter import simpledialog
import socket
import threading


class TelnetApp:
    def __init__(self, root):
        self.root = root
        self.root.title("Telnet")

        #default values for port and host
        self.port = "1234"
        self.ip = "192.168.56.102"

        self.curr_command = ""
        self.quitFlag = 0

        self.setup_ui()


    def setup_ui(self):
        root.protocol("WM_DELETE_WINDOW", self._quit)
        self.root.rowconfigure(0, minsize=200, weight=1)
        self.root.rowconfigure(1, minsize=40, weight=0)
        self.root.columnconfigure(1, minsize=800, weight=1)
        self.txt_edit = tk.Text(self.root)
        self.txt_input = tk.Entry(self.root)

        frm_buttons = tk.Frame(self.root, relief=tk.RAISED, bd=2)
        frm_buttons.grid(row=0, column=0, sticky="ns")
        self.txt_edit.grid(row=0, column=1, sticky="nsew")

        self.txt_input.grid(row=1, column=1, sticky="nsew")
        self.txt_input.bind("<Return>", (lambda event: self.get_command(self.txt_input.get())))

        self.txt_edit.config(state=tk.DISABLED)

        # Create a menu bar
        menubar = tk.Menu(self.root)
        menu = tk.Menu(menubar, tearoff=0)
        menu.add_command(label="Set Ip", command=lambda: self.set_ip())
        menu.add_command(label="Set Port", command=lambda: self.set_port())

        #start setting up connection in thread so that tkinter gui does not freeze and remains responsive
        menu.add_command(label="Connect", command = lambda: threading.Thread(target = self.setup_connection).start())
        menubar.add_cascade(label="Menu", menu=menu)

        # Attach the menu bar to the window
        self.root.config(menu=menubar)

    def set_port(self):
        self.port = simpledialog.askstring(title="Port",prompt="Input port number:",
                                           initialvalue=self.port)

        print(f"Port set to: {self.port}")
        self.print_on_console(f"Port set to: {self.port}")

    def set_ip(self):
        self.ip = simpledialog.askstring(title="Ip Address",prompt="Input ip address:",
                                         initialvalue=self.ip)
        print(f"IP set to: {self.ip}")
        self.print_on_console(f"IP set to: {self.ip}")

    def get_command(self,command):
        if not self.special_command(command):
            self.curr_command = command

    def special_command(self,command):
        """Function used for special commands such as CLEAR
            If special command was used, then it returns 1
        """
        print(command)
        if command == "clear":
            print(command)
            self.txt_edit.config(state=tk.NORMAL)
            self.txt_edit.delete('1.0', tk.END)
            self.clear_curr_command()
            self.txt_edit.config(state=tk.DISABLED)
            return 1
        return 0

    def clear_curr_command(self):
        self.curr_command = ""
        self.txt_input.delete(0, 'end')

    def print_on_console(self,msg):
        self.txt_edit.config(state=tk.NORMAL)
        self.txt_edit.insert("end", msg + "\n")
        self.txt_edit.config(state=tk.DISABLED)

    def setup_connection(self):
        """Function to connect to the server and start threads"""
        self.port = int(self.port)
        with socket.socket(socket.AF_INET, socket.SOCK_STREAM) as s:
            try:
                s.connect((self.ip, self.port))
            except socket.error as exc:
                print("Caught exception socket.error : %s" % exc)
                self.print_on_console(str(exc))
                return

            connect_message = f"Connected to server at {self.ip}:{self.port}"
            print(connect_message)
            self.print_on_console(connect_message)
            # Create threads for sending and receiving messages
            send_thread = threading.Thread(target=self.send_messages, args=(s,))
            receive_thread = threading.Thread(target=self.receive_messages, args=(s,))

            send_thread.daemon = True
            receive_thread.daemon = True

            # Start both threads
            send_thread.start()
            receive_thread.start()

            # Wait for threads to complete
            send_thread.join()
            receive_thread.join()

    def send_messages(self,sock):
        """Function to send messages to the server."""
        while True:
            if self.quitFlag == 1:
                # Shutting down the program
                sock.shutdown(socket.SHUT_RDWR)
                break
            if len(self.curr_command)>0:
                try:
                    message = self.curr_command
                    self.print_on_console("<" + message + ">")
                    message = message.encode()
                    sock.sendall(message)
                    self.clear_curr_command()

                except Exception as e:
                    print(f"Error sending message: {e}")
                    self.print_on_console(f"Error sending message: {e}")
                    break

    def receive_messages(self,sock):
        """Function to receive messages from the server."""
        while True:
            if self.quitFlag == 1:
                # shutting down the socket and program
                sock.shutdown(socket.SHUT_RDWR)
                break
            try:
                data = sock.recv(4096)  # Use a larger buffer size for multiple lines
                if not data:
                    # Cannot send it to GUI console because then this thread will refuse to stop
                    print("Connection closed by server")
                    sock.close()
                    break
                # Decode and split into lines, then print each line
                response = data.decode()
                lines = response.splitlines()
                self.txt_edit.config(state=tk.NORMAL)
                for line in lines:
                    self.txt_edit.insert("end",line+"\n")
                    self.txt_edit.see("end")
                    print(f"Received: {line}")
                self.txt_edit.config(state=tk.DISABLED)
            except Exception as e:
                print(f"Error receiving data: {e}")
                self.print_on_console(f"Error receiving data: {e}")
                break

    def _quit(self):
        # Shutting down the program
        self.quitFlag=1
        self.root.quit()
        self.root.destroy()

if __name__ == "__main__":
    root = tk.Tk()
    app = TelnetApp(root)
    root.mainloop()
    print("Closed program")
