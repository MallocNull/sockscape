﻿using System;
using System.Collections.Generic;
using System.Linq;
using System.Net.Sockets;
using System.Text;
using System.Threading;

namespace Kneesocks {
    public class Stack<T> where T : Connection {
        private Pool<T> PoolRef = null;
        private List<Connection> Clients = new List<Connection>();
        private bool RunWithNoClients = false;
        private bool Running = true;
        private bool _finished = false;

        public Stack(Pool<T> poolRef, Connection initialConnection = null) {
            PoolRef = poolRef;
            if(initialConnection != null)
                Clients.Add(initialConnection);
        }

        public Stack(Pool<T> poolRef, bool runWithNoClients, Connection initialConnection = null)
            : this(poolRef, initialConnection) 
        {
            RunWithNoClients = runWithNoClients;
        }

        public void AddClient(Connection client) {
            lock(Clients) {
                Clients.Add(client);
            }
        }

        public int Count {
            get {
                return Clients.Count;
            }
        }

        public void StopThread() {
            Running = false;
        }

        public bool Finished { get; private set; }

        // USED FOR THREADING -- DO NOT CALL
        public void ManageStack() {
            while(Running && (Count > 0 || RunWithNoClients)) {
                for(var i = Count - 1; i >= 0 && Running; --i) {
                    var client = Clients[i];
                    client.Parse();
                }
            }

            Finished = true;
            PoolRef.InvalidateThread(this);
        }
    }
}