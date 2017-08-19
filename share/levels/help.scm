;; This level contains the scenes used in the help menu
(set-track-name "help")
(set-author "msto")
(add-flag 248 250 50 #t 0.1)
(fog 1)

;; Helper macros...
(define step 16.)
(define dx (lambda (s) (+ s (* xoff step))))
(define dy (lambda (s) (+ s (* yoff step))))

;; Exhibit A -- the Player

(define xoff 0)
(define yoff 0)

(set-start-position (dx 4) (dy 4))

;; Exhibit B -- the Mr Blacks

(define xoff 0)
(define yoff 1)

(define fenway (new-mr-black (dx 2.5) (dy 4.5)))
(define qualbert (new-mr-black (dx 3.5) (dy 3.5)))
(define justeaze (new-mr-black (dx 4.5) (dy 2.5)))
(set-primary-color fenway 0.0 0.0 0.6 1.0)
(set-primary-color qualbert 0. 0. 0. 1.0)
(set-primary-color justeaze 1. 0.8 0.8 1.0)
(set-modtime qualbert *mod-spike* -1.)
(set-modtime justeaze *mod-small* -1.)
(set-modtime justeaze *mod-nitro* -1.)

(define metchley (new-mr-black (dx 5) (dy 5)))
(set-modtime metchley *mod-large* -1.)
(set-primary-color metchley 0.0 0.8 0.0 1.0)
(set-texture metchley "track.png")


;; Exhibit C -- Spike modpill

(define xoff 1)
(define yoff 0)

(add-modpill (dx 4) (dy 4) *mod-spike* 3 2)


;; Exhibit D -- Speed modpill

(define xoff 1)
(define yoff 1)

(add-modpill (dx 4) (dy 4) *mod-speed* 3 2)

;; Exhibit E -- Freeze modpill

(define xoff 2)
(define yoff 0)

(add-modpill (dx 4) (dy 4) *mod-frozen* 3 2)


;; Exhibit F -- Glass modpill

(define xoff 2)
(define yoff 1)

(add-modpill (dx 4) (dy 4) *mod-glass* 3 2)

;; Exhibit G -- Nitro modpill

(define xoff 3)
(define yoff 0)

(add-modpill (dx 4) (dy 4) *mod-nitro* 3 2)


;; Exhibit H -- Switch

(define xoff 3)
(define yoff 1)

(define nop (lambda () #f))
(set-primary-color (set-onoff (switch (dx 4.5) (dy 3.5) nop nop) #t) 0.5 0.5 0.5 1.0)
(set-primary-color (set-onoff (switch (dx 3.5) (dy 4.5) nop nop) #f) 0.5 0.5 0.5 1.0)
(set-primary-color (set-onoff (switch (dx 3.5) (dy 3.5) nop nop) #f) 1.0 1.0 0.2 1.0)
(set-primary-color (set-onoff (switch (dx 4.5) (dy 4.5) nop nop) #t) 0.2 1.0 1.0 1.0)


;; Exhibit I -- Spike

(define xoff 4)
(define yoff 0)

(add-sidespike (dx 3) (dy 4) 0.3 0.5 2)
(add-sidespike (dx 4) (dy 3) 0.2 0.4 4)
(add-spike (dx 4) (dy 4) 0.1 0.3)




;; Exhibit J -- Flag

(define xoff 4)
(define yoff 1)

(set-primary-color (add-flag (dx 4) (dy 4) 50 #t 0.1) 0.0 1.0 0.0)
(set-primary-color (add-flag (dx 3) (dy 5) 50 #t 0.1) 0.0 0.0 1.0)
(set-primary-color (add-flag (dx 5) (dy 3) 50 #t 0.1) 1.0 0.0 0.0)



;; Exhibit N -- Goal

(define xoff 7)
(define yoff 0)

(add-goal (dx 3.2) (dy 4) #f "")
(add-goal (dx 4) (dy 3.2) #t "")


;; Exhibit O -- Force field

(define xoff 7)
(define yoff 1)

(define ff1 (forcefield (dx 3) (dy 3.5) 0 0.0 1.0 0.0 1.0 *ff-bounce*))
(set-primary-color ff1 0. 0. 1. 1.0)
(define ff2 (forcefield (dx 5) (dy 3.5) 0 0.0 1.0 0.0 1.0 *ff-bounce*))
(set-primary-color ff2 0. 1. 0. 1.0)


;; Exhibit P -- Cactus

(define xoff 8)
(define yoff 0)

(add-cactus (dx 3) (dy 5) 0.35)
(add-cactus (dx 4) (dy 4) 0.55)
(add-cactus (dx 5) (dy 3) 0.75)



;; Exhibit Q -- Bird

(define xoff 8)
(define yoff 1)

(add-bird (dx 3.3) (dy 4.5) (dx 4.5) (dy 2.7) 0.4 1.5)



;; Exhibit R -- Teleport

(define xoff 9)
(define yoff 0)

(add-teleport (dx 4) (dy 4) 224 252 0.3)


;; Exhibit S -- Tunnel/Pipe

(define xoff 9)
(define yoff 1)

(set-primary-color (pipe (dx 3) (dy 4.5) -6.1 (dx 4) (dy 5.0) -6.5 0.30) 1.0 1.0 1.0 1.0)
(set-primary-color (pipe (dx 2.3) (dy 3) -7.0 (dx 5.5) (dy 3) -7.0 0.45) 0.5 0.5 0.5 0.9)

