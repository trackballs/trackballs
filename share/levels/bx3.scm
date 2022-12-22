;;; azteca

(day)
(set-track-name (_ "Azteca"))
(set-author "Warren D Ober")
(start-time 1140) 
(set-start-position 97 176)


(trigger 125 122 1.0 (lambda ()
(play-effect "capuchmonk.wav")
))
(play-effect "capuchmonk.wav")
;;Flags.

(add-flag 118 87 100 #t 0.1)
(add-flag 118 89 100 #t 0.1)
(add-flag 118 91 100 #t 0.1)
(add-flag 118 93 100 #t 0.1)
(add-flag 118 95 100 #t 0.1)
(add-flag 118 97 100 #t 0.1)
(add-flag 120 97 100 #t 0.1)
(add-flag 122 97 100 #t 0.1)
(add-flag 124 97 100 #t 0.1)
(add-flag 126 97 100 #t 0.1)
(add-flag 128 97 100 #t 0.1)
(add-flag 130 97 100 #t 0.1)
(add-flag 130 95 100 #t 0.1)
(add-flag 130 93 100 #t 0.1)
(add-flag 130 91 100 #t 0.1)
(add-flag 130 89 100 #t 0.1)
(add-flag 130 87 100 #t 0.1)
(add-flag 128 87 100 #t 0.1)
(add-flag 126 87 100 #t 0.1)
(add-flag 124 87 100 #t 0.1)

(add-flag 122 74 250 #t 0.1)
(add-flag 122 72 250 #t 0.1)
(add-flag 122 70 250 #t 0.1)
(add-flag 122 68 250 #t 0.1)

(add-flag 49 100 100 #t 0.1)
(add-flag 49 98 100 #t 0.1)
(add-flag 53 90 100 #t 0.1)
(add-flag 53 82 100 #t 0.1)
(add-flag 53 74 100 #t 0.1)
(add-flag 53 66 100 #t 0.1)
(add-flag 53 58 100 #t 0.1)

(add-flag 80 65 100 #t 0.1)
(add-flag 80 73 100 #t 0.1)
(add-flag 80 89 100 #t 0.1)
(add-flag 80 97 100 #t 0.1)
(add-flag 45 73 100 #t 0.1)
(add-flag 45 71 100 #t 0.1)
(add-flag 45 69 100 #t 0.1)
(add-flag 45 69 100 #t 0.1)

(add-flag 87 51 100 #t 0.1)
(add-flag 90 51 100 #t 0.1)
(add-flag 93 51 100 #t 0.1)
(add-flag 96 51 100 #t 0.1)
(add-flag 99 51 100 #t 0.1)

;Diamonds

;1st pyramid.
(diamond 126 122)
;Upper complex.
(diamond 63 48)
;Arena.
(diamond 80 81)
;Outer complex.
(diamond 118 53)
;near pit.
(diamond 67 133)


;;Xtra balls.

(add-modpill 124 171 *mod-extra-life* 30 -1)
(add-modpill 73.5 117.5 *mod-extra-life* 30 -1)

;;1st pyramid entrance.

(define pipe1 (pipe 112.1 124.5 -3.5 122 124.5 -3.5 0.5))
(set-primary-color pipe1 0.1 0.1 0.1 1.0)
(pipe-connector 122 124.5 -3.5 0.6)
(define pipe2 (pipe 122.0 124.5 -3.5 124.0 124.5 2.0 0.5))
(set-wind pipe2 8.0 0.0)
(set-primary-color (pipe-connector 123.9 124.5 2.0 0.6) 0.1 0.1 0.1 1.0)
(define pipe3 (pipe 123.9 124.5 2.0 122.9 124.5 5.4 0.5))
(set-wind pipe3 8.0 0.0)
(set-primary-color pipe3 0.1 0.1 0.1 1.0)
(set-primary-color (pipe-connector 122.9 124.5 5.4 0.6) 0.1 0.1 0.1 1.0)
(define pipe4 (pipe 122.9 124.5 5.4 121.2 124.5 5.1 0.5))
(set-wind pipe4 5.5 0.0)
(set-primary-color pipe4 0.1 0.1 0.1 1.0)



;; Trigger to access complex.

(define blinker1 (add-colormodifier 3 128 124 0.5 1.0 0.5 1.0))
(trigger-once 128 124 0.5 (lambda ()
  (set-cell-heights 122 107 130 107 -3.5 -3.5 -3.5 -3.5)
  (set-cell-heights 122 106 130 106 -3.5 -3.5 -3.5 -3.5)
  (set-cell-heights 122 106 130 103 -3.0 -3.0 -3.0 -3.0)
  (set-cell-heights 123 104 129 103 -2.5 -2.5 -2.5 -2.5)
  (set-cell-heights 124 103 128 103 -2.0 -2.0 -2.0 -2.0)
  (set-primary-color (sign (_ "You have access to the complex.") 0.8 1.0 -0.1 128 124 8.5) 0.2 0.5 1.0 1.0)
  (set-onoff blinker1 #f)
  ;(set-onoff tempsign #f)
))


;; Trigger to access 2nd pyramid.

(define blinker2 (add-colormodifier 3 119 54 0.5 1.0 0.5 1.0))
(trigger-once 119 54 0.5 (lambda ()
  (set-cell-heights 87 119 101 119 -8.0 -8.0 -8.0 -8.0)
  (set-cell-heights 87 118 101 118 -8.0 -8.0 -8.0 -8.0)
  (set-cell-heights 87 117 101 117 -8.0 -8.0 -8.0 -8.0)
  (set-cell-heights 87 116 101 116 -8.0 -8.0 -8.0 -8.0)
  (set-cell-flag 87 116 101 119  *cell-sand* #f)
  (set-cell-flag 87 116 101 119  *cell-nogrid* #f)
  (set-cell-flag 79 119 86 116  *cell-acid* #t)
  (set-cell-flag 79 119 86 116  *cell-nogrid* #t)
  (set-primary-color (sign (_ "The way is opened across the sand.") 0.8 1.0 -0.1 119 54) 0.2 0.5 1.0 1.0)
  (set-onoff blinker2 #f)
))


;; Trigger to access Arena.

(define blinker3 (add-colormodifier 3 64 118 0.5 1.0 0.5 1.0))
(trigger-once 64 118 0.5 (lambda ()
  (set-cell-heights 57 102 77 102 -7.0 -7.0 -7.0 -7.0)
  (set-primary-color (sign (_ "You must enter the Arena.") 0.8 1.0 -0.1 64 118) 0.2 0.5 1.0 1.0)
  (set-onoff blinker3 #f)
))

;; Triggers to raise temple slide.

(trigger-once 76 117 0.5 (lambda ()
  (set-cell-heights 77 116 77 116 -5.0 -5.0 -5.5 -5.5)
  (set-cell-heights 77 117 77 117 -4.5 -5.0 -5.0 -5.5)
  (set-cell-heights 77 118 77 118 -5.0 -4.5 -5.5 -5.0)
  (set-cell-heights 77 119 77 119 -5.0 -5.0 -5.5 -5.5)
  (set-cell-heights 78 116 78 116 -5.5 -5.5 -6.0 -6.0)
  (set-cell-heights 78 117 78 117 -5.0 -5.5 -5.5 -6.0)
  (set-cell-heights 78 118 78 118 -5.5 -5.0 -6.0 -5.5)
  (set-cell-heights 78 119 78 119 -5.5 -5.5 -6.0 -6.0)
  (set-cell-flag 79 119 86 116  *cell-acid* #f)
  (set-cell-flag 79 119 86 116  *cell-sand* #t)
  (set-cell-flag 79 119 86 116  *cell-nogrid* #f)
))
(trigger-once 76 118 0.5 (lambda ()
  (set-cell-heights 77 116 77 116 -5.0 -5.0 -5.5 -5.5)
  (set-cell-heights 77 117 77 117 -4.5 -5.0 -5.0 -5.5)
  (set-cell-heights 77 118 77 118 -5.0 -4.5 -5.5 -5.0)
  (set-cell-heights 77 119 77 119 -5.0 -5.0 -5.5 -5.5)
  (set-cell-heights 78 116 78 116 -5.5 -5.5 -6.0 -6.0)
  (set-cell-heights 78 117 78 117 -5.0 -5.5 -5.5 -6.0)
  (set-cell-heights 78 118 78 118 -5.5 -5.0 -6.0 -5.5)
  (set-cell-heights 78 119 78 119 -5.5 -5.5 -6.0 -6.0)
  (set-cell-flag 79 119 86 116  *cell-acid* #f)
  (set-cell-flag 79 119 86 116  *cell-sand* #t)
  (set-cell-flag 79 119 86 116  *cell-nogrid* #f)
))



;;Opponents in arena

(define Mr1 (new-mr-black 82 85))
(set-acceleration Mr1 5.0)
(set-horizon Mr1 15.0)
(set-primary-color Mr1 0.9 0.4 0.0 1.0)
(define Mr2 (new-mr-black 92 75))
(set-acceleration Mr2 5.0)
(set-horizon Mr2 15.0)
(set-primary-color Mr2 0.2 0.9 0.2 1.0)
(define Mr3 (new-mr-black 82 65))
(set-primary-color Mr1 0.9 0.4 0.0 1.0)
(set-acceleration Mr3 5.0)
(set-horizon Mr3 10.0)
(set-primary-color Mr3 0.9 0.3 0.9 1.0)


(add-modpill 82 82 *mod-speed* 50 50)
(add-modpill 104 82 *mod-jump* 50 50)

;;Triggers in Arena.

(define blinker4 (add-colormodifier 3 103 73 0.5 1.0 0.5 1.0))
(trigger-once 103 73 0.5 (lambda ()
  (set-cell-heights 103 61 104 53 -3.5 -3.5 -3.5 -3.5)
  (set-primary-color (sign (_ "A wall has been lowered.") 0.8 1.0 -0.1 103 73) 0.2 0.5 1.0 1.0)
  (set-onoff blinker4 #f)
))

(define blinker5 (add-colormodifier 3 103 92 0.5 1.0 0.5 1.0))
(trigger-once 103 92 0.5 (lambda ()
  (set-cell-heights 115 93 116 91 -1.5 -1.5 -1.5 -1.5)
  (set-primary-color (sign (_ "A wall has been lowered.") 0.8 1.0 -0.1 103 92) 0.2 0.5 1.0 1.0)
  (set-onoff blinker5 #f)
))

(define blinker6 (add-colormodifier 3 83 73 0.5 1.0 0.5 1.0))
(trigger-once 83 73 0.5 (lambda ()
  (set-cell-heights 66 80 72 66 -0.5 -0.5 -0.5 -0.5)
  (set-primary-color (sign (_ "A roof has been raised.") 0.8 1.0 -0.1 83 73) 0.2 0.5 1.0 1.0)
  (set-onoff blinker6 #f)
))

(define blinker7 (add-colormodifier 3 83 92 0.5 1.0 0.5 1.0))
(trigger-once 83 92 0.5 (lambda ()
  (set-cell-heights 76 146 86 146 -9.0 -9.0 -9.0 -9.0)
  (set-cell-heights 76 147 86 147 -8.5 -8.5 -8.5 -8.5)
  (set-cell-heights 76 148 86 148 -8.0 -8.0 -8.0 -8.0)
  (set-cell-heights 76 149 86 149 -7.5 -7.5 -7.5 -7.5)
  (set-cell-heights 76 150 86 150 -7.0 -7.0 -7.0 -7.0)
  (set-cell-heights 76 151 86 151 -6.5 -6.5 -6.5 -6.5)
  (set-cell-heights 80 152 80 154 -4.5 -4.5 -4.5 -4.5)
  (set-cell-heights 81 152 81 154 -5.0 -5.0 -5.0 -5.0)
  (set-cell-heights 82 152 82 154 -5.5 -5.5 -5.5 -5.5)
  (set-cell-heights 83 152 86 154 -6.0 -6.0 -6.0 -6.0)
  (set-primary-color (sign (_ "Stairs are in place.") 0.8 1.0 -0.1 83 92) 0.2 0.5 1.0 1.0)
  (set-onoff blinker7 #f)
))

(define blinker8 (add-colormodifier 3 75 162 0.5 1.0 0.5 1.0))
(trigger-once 75 162 0.5 (lambda ()
  (set-cell-heights 59 53 66 53 -4.0 -4.0 -4.0 -4.0)
  (set-cell-heights 59 52 66 52 -3.5 -3.5 -3.5 -3.5)
  (set-cell-heights 59 51 66 51 -3.0 -3.0 -3.0 -3.0)

  (set-primary-color (sign (_ "You may enter upper complex.") 0.8 1.0 -0.1 75 162) 0.2 0.5 1.0 1.0)
  (set-onoff blinker8 #f)
))

(set-primary-color (pipe 95 166 -2.5 99 166 -2.5 1.5) 0.1 0.1 0.1 1.0)

(add-teleport 133 151 122 71 0.3)

;;Trigger to enter final temple.

(define blinker9 (add-colormodifier 3 53 101 0.5 1.0 0.5 1.0))
(trigger-once 53 101 0.5 (lambda ()
  (set-cell-heights 119 77 125 77 4.0 4.0 4.0 4.0)
  (set-primary-color (sign (_ "The exit is now open.") 0.8 1.0 -0.1 53 101) 0.2 0.5 1.0 1.0)
  (set-onoff blinker9 #f)
))



;; Goal!
(add-goal 126 92 #t "")


