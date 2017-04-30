(set-track-name "motion1")
(set-author "Warren D. Ober")
(set-start-position  144 125)  
(start-time 720)
(restart-time 90)
(map-is-transparent #t)

;; SET JUMP TO Default;

(jump 1.0)

;;Weather conditions.

(day)



;;Set ball velocity.

(define speed 0.3)
(cond
 ((= (difficulty) *easy*) (set! speed 0.20))
 ((= (difficulty) *normal*) (set! speed 0.30))
 ((= (difficulty) *hard*) (set! speed 0.40)))


;;Flags.

(add-flag 14 18 350 1 0.1)
(add-flag 14 26 150 1 0.1)
(add-flag 17 26 350 1 0.1)
(add-flag 16 40 200 1 0.1)





;;Platform

(add-cyclic-platform 
 ;;; Positions
101 80 102 81 
 ;;; Low, High, Time Offset, Speed
 -8.0 5. 0.0 0.75)






;;PIPES 

;scoop

(define pipe1(pipe 102 81.5 4.8 102 83.5 8.5 1.4))
(set-primary-color pipe1 0.5 0.0 0.0 0.3)
(set-wind pipe1 15.0 0.0)

(set-primary-color (pipe-connector 102.0 83.5 8.5 1.4) 0.5 0.0 0.0 0.3)

(define pipe2(pipe 102.0 83.5 8.7 102.0 100.0 8.7 1.0))
(set-primary-color pipe2 0.5 0.0 0.0 0.3)
(set-wind pipe2 3.5 0.0)


;1st turn.
 
(set-primary-color (pipe-connector 102.0 100.5 8.7 1.1) 0.5 0.0 0.0 0.3)

(define pipe3(pipe 102.1 100.4 8.6 114.9 88.4 1.2 1.0))
(set-primary-color pipe3 0.5 0.0 0.0 0.3)
(set-wind pipe3 0.0 1.0)

;2nd turn.

(set-primary-color (pipe-connector 115.2 88.5 1.0 1.1) 0.5 0.0 0.0 0.3)

(define pipe4(pipe 115.2 88.4 1.0 115.2 100.2 -3.5 1.0))
(set-primary-color pipe4 0.5 0.0 0.0 0.3)
(set-wind pipe4 2.0 0.0)

;3rd turn. (up)

(set-primary-color (pipe-connector 115.2 100.5 -3.4 1.2) 0.5 0.0 0.0 0.3)

(define pipe5(pipe 114.9 100.5 -3.6 134.9 100.6 7.6 1.0))
(set-primary-color pipe5 0.5 0.0 0.0 0.3)
(set-wind pipe5 6.5 0.0)

;4th turn.

(set-primary-color (pipe-connector 135.2 100.6 7.4 1.0) 0.5 0.0 0.0 0.3)

(define pipe6(pipe 135.2 100.5 7.4 135.2 120.1 7.4 1.0))
(set-primary-color pipe6 0.5 0.0 0.0 0.3)
(set-wind pipe6 3.0 0.0)

;last turn (down to shute).

(set-primary-color (pipe-connector 135.2 120.5 7.5 1.0) 0.5 0.0 0.0 0.3)

(define pipe7(pipe 135.3 120.5 7.5 107.0 114.5 -4.0 1.0))
(set-primary-color pipe7 0.5 0.0 0.0 0.3)
(set-wind pipe7 1.0 0.0)

;Drop shute

(define pipe8(pipe 104.5 114.5 -7.5 104.0 114.5 -9.5 1.5))
(set-primary-color pipe8 0.9 0.9 0.9 0.9)
(set-wind pipe8 -10.0 0.0)

;connector


(set-primary-color (pipe-connector 104.0 114.5 -9.5 1.5) 0.9 0.9 0.9 0.9)

(define pipe9(pipe 104.0 114.8 -9.8 102.5 121.5 -9.8 1.0))
(set-primary-color pipe8 0.9 0.9 0.9 0.9)
(set-wind pipe8 0.0 15.0)

(set-primary-color (pipe-connector 102.5 121.5 -9.8 1.0) 0.9 0.9 0.9 0.9)

;;BEGIN LIFT SHUTE

(define shute(pipe 102.5 121.5 -9.8 102.5 122.5 16.4 1.0))
(set-primary-color shute 0.9 0.9 0.9 0.3)
(set-wind shute 4.0 5.0)



;;End of level

(add-goal 34 60 #f "bx3")


