﻿using System;
using System.Collections.Generic;
using System.ComponentModel.DataAnnotations;
using System.Linq;
using System.Text;
using System.Threading.Tasks;

namespace CircleScape.DAL {
    public partial class User {
        public long Id { get; set; }

        [Required]
        public string Username { get; set; }

        [Required]
        public string Password { get; set; }

        [Required]
        public DateTime Joined { get; set; }
        public DateTime? LastLogin { get; set; }

        public virtual ICollection<Origin> Origins { get; set; }
    }
}