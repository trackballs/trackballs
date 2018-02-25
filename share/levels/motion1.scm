(set-track-name (_ "Pendulum Motion"))
(set-author "Warren D. Ober")
(set-start-position  143.5 124.5)  
(start-time 720)
(restart-time 90)

;; SET JUMP TO Default;

(jump 1.0)

;;Weather conditions.

(day)

;;Flags.

(add-flag 14 18 350 #t 0.1)
(add-flag 14 26 150 #t 0.1)
(add-flag 17 26 350 #t 0.1)
(add-flag 16 40 200 #t 0.1)


;; Pit guardian
(define pit-boss (new-mr-black 125 105))
(cond
 ((= (difficulty) *easy*) (set-acceleration pit-boss 2.0))
 ((= (difficulty) *normal*) (set-acceleration pit-boss 4.0))
 ((= (difficulty) *hard*) (set-acceleration pit-boss 7.0)))
(set-horizon pit-boss 15.0)
(set-primary-color pit-boss 1.0 1.0 1.0 1.0)
(set-modtime pit-boss *mod-spike* -1.)


;;Platform

(add-cyclic-platform 
 ;;; Positions
101 80 102 81 
 ;;; Low, High, Time Offset, Speed
 -8.0 5. 0.0 0.95)


(define pipe-path 
  '((101.5 80.8  5.1)
    (101.5 81.8  8.6)
    (101.5 100.0 8.2)
    (114.5 88.0  1.0)
    (112.5 97.0 -3.5)
    (116.5 103.0 -3.5)
    (134.5 100.0 7.5)
    (133.5 118.0 6.5)
    (108.5 115.0 -4.0)))

(map (lambda (p) (set-primary-color (set-wind p 10. 10.) 0.5 0.0 0.0 0.3))
     (multi-pipe pipe-path 1.3 #t))


;scoop

;Drop shute

(define pipe8 (pipe 104 114 -7.5 103.5 114.2 -9.5 1.5))
(set-primary-color pipe8 0.9 0.9 0.9 0.9)
(set-wind pipe8 5.0 5.0)

;connector

(set-primary-color (pipe-connector 103.5 114 -9.5 1.5) 0.9 0.9 0.9 0.9)

(define pipe9 (pipe 103.5 112.3 -9.8 103.5 119 -9.8 1.4))
(set-primary-color pipe9 0.9 0.9 0.9 0.9)
(set-wind pipe9 10.0 10.0)

(set-primary-color (pipe-connector 103.5 119 -9.8 1.3) 0.9 0.9 0.9 0.9)

(define pipe10 (pipe 103.5 118 -9.8 102 121.5 -8.5 1.2))
(set-primary-color pipe10 0.9 0.9 0.9 0.9)
(set-wind pipe10 10.0 10.0)
(set-primary-color (pipe-connector 102 121.5 -8.5 1.1) 0.9 0.9 0.9 0.9)

; begin lift chute

(define shute (pipe 102 121.5 -10.5 102 121.5 15.4 1.0))
(set-primary-color shute 0.9 0.9 0.9 0.3)
(set-wind shute 10.0 10.0)
(define shute2 (pipe 102 121.5 15.4 102.3 121.5 16.4 1.0))
(set-primary-color shute2 0.9 0.9 0.9 0.3)
(set-wind shute2 3.0 3.0)



;;End of level

(trigger-once 102 122 1.5 (lambda ()
  (add-goal 124 105 #f "")))


