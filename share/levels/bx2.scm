;;; Four Doors

;; Environment Section. (defines Level settings)

(set-track-name (_ "Four Doors"))
(set-author "Warren D. Ober")
(set-start-position 13.5 1.5)     ;try these: 31 44)  82 24)  66 63)
(start-time 720)
(restart-time 90)

;; SET JUMP TO Default;

(jump 1.0)

;;Weather conditions.

(day)
(fog)

;(map (lambda(p) (set-wind p 1.0 -4.0)) p1)

;;Clear local areas of fog


;;Set ball velocity.

(define speed 0.3)
(cond
 ((= (difficulty) *easy*) (set! speed 0.20))
 ((= (difficulty) *normal*) (set! speed 0.30))
 ((= (difficulty) *hard*) (set! speed 0.40)))


;;;Object declaration section. (defines objects.)

;;These four doors must open to exit level.


(define doorff1 (forcefield 28.5 28.5 0.0 5.0 0.0 0.0 6.0 *ff-bounce*))
(switch 15 54 (lambda () (set-onoff doorff1 #f)) (lambda () (set-onoff doorff1 #f)))

(define doorff2 (forcefield 28.5 30.5 0.0 5.0 0.0 0.0 6.0 *ff-bounce*))
(switch 31 84 (lambda () (set-onoff doorff2 #f)) (lambda () (set-onoff doorff2 #f)))

(define doorff3 (forcefield 28.5 32.5 0.0 5.0 0.0 0.0 6.0 *ff-bounce*))
(switch 68 42 (lambda () (set-onoff doorff3 #f)) (lambda () (set-onoff doorff3 #f)))

(define doorff4 (forcefield 28.5 34.5 0.0 5.0 0.0 0.0 6.0 *ff-bounce*))
(switch 48 5 (lambda () (set-onoff doorff4 #f)) (lambda () (set-onoff doorff4 #f)))


;;Flags near start.

(add-flag 14 18 350 #t 0.1)
(add-flag 14 26 150 #t 0.1)
(add-flag 17 26 350 #t 0.1)
(add-flag 16 40 200 #t 0.1)

;;Flags by spikes.

(add-flag 9 48 150 #t 0.1)
(add-flag 9 51 250 #t 0.1)
(add-flag 9 55 150 #t 0.1)
(add-flag 9 57 350 #t 0.1)

;;Flags near garden.

(add-flag 15 64 350 #t 0.1)
(add-flag 17 60 250 #t 0.1)
(add-flag 19 66 250 #t 0.1)
(add-flag 16 69 150 #t 0.1)
(add-flag 13 69 350 #t 0.1)
(add-flag 13 67 150 #t 0.1)
(add-flag 12 62 350 #t 0.1)

;;Flags in park.

(add-flag 22 88 350 #t 0.1)
(add-flag 21 90 250 #t 0.1)
(add-flag 33 83 250 #t 0.1)
(add-flag 34 84 150 #t 0.1)

;;Flags in city.

(add-flag 61 63 350 #t 0.1)
(add-flag 66 56 150 #t 0.1)
(add-flag 83 28 350 #t 0.1)
(add-flag 81 37 350 #t 0.1)
(add-flag 78 37 250 #t 0.1)
(add-flag 78 40 250 #t 0.1)
(add-flag 80 40 150 #t 0.1)
(add-flag 80 44 350 #t 0.1)
(add-flag 81 50 250 #t 0.1)
(add-flag 81 54 250 #t 0.1)
(add-flag 76 55 150 #t 0.1)
(add-flag 66 62 350 #t 0.1)
(add-flag 66 66 250 #t 0.1)

;;Flags on Moon.

(add-flag 35 14 350 #t 0.1)
(add-flag 39 8 350 #t 0.1)
(add-flag 44 15 350 #t 0.1)


;;Flags in pyramids.

(add-flag 31 41 1000 #t 0.1)
(add-flag 31 44 1000 #t 0.1)

;;Spikes of doom

(add-sidespike 11 49 speed 0.1 1)
(add-sidespike 11 53 speed 0.1 4)
(add-sidespike 12 49 speed 0.2 4)
(add-sidespike 12 51 speed 0.2 4)
(add-sidespike 14 53 speed 0.1 4)
(add-sidespike 14 52 speed 0.1 2)
(add-sidespike 15 50 speed 0.1 1)
(add-spike 13 49 speed -0.00)
(add-spike 13 50 speed -0.20)
(add-spike 13 51 speed -0.40)
(add-spike 13 52 speed -0.60)
(add-spike 10 50 speed -0.00)
(add-spike 11 50 speed -0.20)
(add-spike 11 51 speed -0.40)
(add-spike 11 52 speed -0.60)
(add-spike 12 53 speed -0.80)
(add-spike 12 54 speed -0.00)
(add-spike 13 54 speed -0.00)
(add-spike 13 53 speed -0.20)
(add-spike 15 53 speed -0.60)
(add-spike 14 49 speed -0.60)
(add-spike 15 49 speed -0.80)
(add-spike 14 51 speed -0.00)
(add-spike 15 51 speed -0.00)

;;Garden Platforms.  x0  y0  x1  y1   Low  Hi  Offset Speed.

(add-cyclic-platform 12  70  12  70  -6.0 -3.5  1.0   0.8)
(add-cyclic-platform 15  69  15  69  -6.0 -3.5  1.5   0.6)
(add-cyclic-platform 18  67  18  67  -6.0 -3.5  2.0   0.7)
(add-cyclic-platform 14  62  14  62  -6.0 -3.5  1.0   1.0)
(add-cyclic-platform 17  62  17  62  -6.0 -3.5  1.5   0.9)


;;These are the guides in the city

;; Each guide consists of one center rail and two side rails;
;; the center rail is **ff-bounce** so we don't fall through from above
;; the side rail forcefields only hold the ball in, not out
(define guide-width 0.4)
(define guide-lift 0.26)
(define center-drop -0.05)
(define center-extend 0.45)
(define rail-height 0.1)
(define ff-bridge-x
  (lambda (x y dx)
    (let ((s (if (> dx 0) -1 1)))
      (forcefield x (- y guide-width) (+ center-drop guide-lift)
                  dx 0.0 0.0
                  rail-height (if (> dx 0) *ff-bounce2* *ff-bounce1*))
      (forcefield (+ x (* s center-extend)) y center-drop
                  (- dx (* s 2 center-extend)) 0.0 0.0
                  rail-height *ff-bounce*)
      (forcefield x (+ y guide-width) (+ center-drop guide-lift)
                  dx 0.0 0.0
                  rail-height (if (> dx 0) *ff-bounce1* *ff-bounce2*)))))
(define ff-bridge-y
  (lambda (x y dy)
    (let ((s (if (> dy 0) -1 1)))
      (forcefield (- x guide-width) y (+ center-drop guide-lift)
                  0.0 dy 0.0
                  rail-height (if (> dy 0) *ff-bounce2* *ff-bounce1*))
      (forcefield x (+ y (* s center-extend)) center-drop
                  0.0 (- dy (* s 2 center-extend)) 0.0
                  rail-height *ff-bounce*)
      (forcefield (+ x guide-width) y (+ center-drop guide-lift)
                  0.0 dy 0.0
                  rail-height (if (> dy 0) *ff-bounce1* *ff-bounce2*)))))

(ff-bridge-x 68.0 52.0 7.0)
(ff-bridge-y 76.0 50.0 -10.0)
(ff-bridge-x 75.0 38.0 -7.0)
(ff-bridge-y 66.0 27.0 10.0)
(ff-bridge-x 67.0 23.0 10.0)

;;2nd set of guides (It is possible!)

(ff-bridge-x 67.0 25.0 14.0)
(ff-bridge-y 64.0 41.0 -16.5)
(ff-bridge-y 64.0 68.0 -25.5)


;;PIPES GROUP 1 (from first raised area to second)

(define pipe2(pipe 84 13 14.0 84 10.1 12.5 0.5))
(set-primary-color pipe2 0.0 0.5 0.0 0.3)

(set-primary-color (pipe-connector 84 10 12.5 0.6) 0.0 0.5 0.0 0.3)

(define pipe3(pipe 84 10 12.5 84 7.1 12.5 0.5))
(set-primary-color pipe3 0.0 0.5 0.0 0.3)
 
(define pipe4(pipe 84 7 12.5 80 7.1 12.5 0.5))
(set-primary-color pipe4 0.0 0.5 0.0 0.3)

(set-primary-color (pipe-connector 84 7 12.5 0.6) 0.0 0.5 0.0 0.3)

(define pipe5(pipe 80 7.1 12.5 77 10 12.5 0.5))
(set-primary-color pipe5 0.0 0.5 0.0 0.3)

(set-primary-color (pipe-connector 80 7 12.5 0.6) 0.0 0.5 0.0 0.3)


;;PIPES GROUP 2 (from second raised area to lower area)

(define pipe6(pipe 73 9.5 12.3 73 4.7 9.7 0.5))
(set-primary-color pipe6 0.0 0.5 0.0 0.3)

(set-primary-color (pipe-connector 73 4.6 9.6 0.6) 0.0 0.5 0.0 0.3)

(define pipe11(pipe 73 4.6 9.5 73 8 7.5 0.5))
(set-primary-color pipe11 0.0 0.5 0.0 0.3)

(set-primary-color (pipe-connector 73 8 7.5 0.6) 0.0 0.5 0.0 0.3)

(define pipe12(pipe 73 8 7.5 70 13 9.0 0.5))
(set-primary-color pipe12 0.0 0.5 0.0 0.3)

(set-primary-color (pipe-connector 70 13 9.0 0.6) 0.0 0.5 0.0 0.3)

(define pipe13(pipe 70 13 9.0 65 13 9.0 0.5))
(set-primary-color pipe13 0.0 0.5 0.0 0.3)


;;PIPES GROUP 3 (from second raised area to upper area)

(define pipe7(pipe 70 12 12.5 66.1 12 12.5 0.5))
(set-primary-color pipe7 0.0 0.5 0.0 0.3)

(define pipe8(pipe 65.9 11.9 12.5 60.2 15.0 14.0 0.5))
(set-primary-color pipe8 0.0 0.5 0.0 0.3)

(set-primary-color (pipe-connector 65.9 11.9 12.5 0.6) 0.0 0.5 0.0 0.3)

(define pipe9(pipe 60.1 15.0 14.0 56 10 14.0 0.5))
(set-primary-color pipe9 0.0 0.5 0.0 0.3)

(set-primary-color (pipe-connector 60 15 14.0 0.6) 0.0 0.5 0.0 0.3)

(define pipe10(pipe 56 10 14.0 50.5 6 14.5 0.5))
(set-primary-color pipe10 0.0 0.5 0.0 0.3)

;Pipes group 4 (from upper raised area to moon.)

(define pipe14(pipe 55.1 13.2 5.7 51 13.2 3.5 0.5))
(set-primary-color pipe14 0.0 0.5 0.0 0.3)

(set-primary-color (pipe-connector 51.0 13.2 3.5 0.6) 0.0 0.5 0.0 0.3)

(define pipe15(pipe 51.0 13.2 3.5 44.5 13.5 -3.5 0.5))
(set-primary-color pipe15 0.0 0.5 0.0 0.3)

(set-primary-color (pipe-connector 44.5 13.5 -3.5 0.6) 0.0 0.5 0.0 0.3)

(define pipe16(pipe 44.5 13.5 -3.5 40.5 13.5 -3.5 0.5))
(set-primary-color pipe16 0.0 0.5 0.0 0.3)



;;Bumpers in Starship Green-rooms.
   
(forcefield 29.5 45.5 0.2  3.0 0.0 0.0 0.1 *ff-bounce*)
(forcefield 32.5 45.5 -3.3  0.0 -3.0 0.0 0.1 *ff-bounce*)
(forcefield 32.5 42.5 0.2 -3.0 0.0 0.0 0.1 *ff-bounce*)
(forcefield 29.5 42.5 0.2  0.0 3.0 0.0 0.1 *ff-bounce*)

(forcefield 29.5 42 0.2  3.0 0.0 0.0 0.1 *ff-bounce*)
(forcefield 32.5 42 0.2  0.0 -3.0 0.0 0.1 *ff-bounce*)
(forcefield 32.5 39 0.2 -3.0 0.0 0.0 0.1 *ff-bounce*)
(forcefield 29.5 39 0.2  0.0 3.0 0.0 0.1 *ff-bounce*)


;;Pyramids of forcefields (Greenrooms).

;(forcefield rX   rY    rZ rDx  rDy rDz rHeight “flag”)

(forcefield 29 45.0 -3.5 2.0 -2.0 3.5 3.5 *ff-bounce*)
(forcefield 33 45.0 -7.0 -2.0 -2.0 3.5 3.5 *ff-bounce*)
(forcefield 33 41.0 -3.5 -2.0 2.0 3.5 3.5 *ff-bounce*)
(forcefield 29 41.0 -3.5 2.0 2.0 3.5 3.5 *ff-bounce*)

(forcefield 29 42.5 -3.5 2.0 -2.0 3.5 3.5 *ff-bounce*)
(forcefield 33 42.5 -3.5 -2.0 -2.0 3.5 3.5 *ff-bounce*)
(forcefield 33 38.5 -3.5 -2.0 2.0 3.5 3.5 *ff-bounce*)
(forcefield 29 38.5 -3.5 2.0 2.0 3.5 3.5 *ff-bounce*)



;;Procedure Section. (Gameplay starts.)


(set-primary-color (sign (_ "4 hidden switches open exit doors.") 0.8 0.0 -0.1 14 22) 0.2 0.5 1.0 1.0)

(set-primary-color (sign (_ "A switch is nearby.") 0.8 0.0 -0.1 13 48) 0.2 0.5 1.0 1.0)

(set-primary-color (sign (_ "Peace.") 0.8 0.0 -0.1 15 66) 0.2 0.5 1.0 1.0)
 

;;Extra life near start.

(add-modpill 18 34 *mod-extra-life* 30 -1)


;;Made it to the Garden! Save your place.


(diamond 9 59)
(trigger 8.5 58.5 1.0 (lambda () (fog)))
(trigger 9.2 59.2 1.0 (lambda () (jump 1.0)))


;;A bird flys by in Park.

(add-bird 11 95 50 60 0.5 2)


;;A floater for the pond jump.

(add-modpill 23 81 *mod-float* 60 -1)


;;This is the water pipe.

(pipe 31 81 -7.5 27.5 81 -7 1.0)


;;Mr. Black in Pipe

(define mr-black1 (new-mr-black 28 81))
(set-acceleration mr-black1 2.0)


;;Extra life Near Switch.

(add-modpill 35 81 *mod-extra-life* 30 -1)



;;Welcome to the city. Save your place here.

(diamond 60 70)
(trigger 59.5 69.5 1.0 (lambda () (fog)))
(trigger 59.8 69.8 1.0 (lambda () (jump 1.0)))


;;Made it through guides, save your place again.

(diamond 80 24)
(trigger 79.5 23.5 1.0 (lambda () (fog)))
(trigger 80.2 24.2 1.0 (lambda () (jump 1.0)))

;;Up to next level

(add-cyclic-platform 
 ;;; Positions
80 21 81 22 
 ;;; Low, High, Time Offset, Speed
 -3. 7. 1. 1.0)



;;Extra life By Platform.

(add-modpill 82 27 *mod-extra-life* 30 -1)


;;BEGIN LIFT SHUTE

(define shute(pipe 84 24 7.5 84 23.1 15.4 0.5))
(set-primary-color shute 0.0 0.5 0.0 0.3)
(set-wind shute 9.0 0.0)

(define pipe1(pipe 84 22.9 15.6 84 20.8 15.5 0.5))
(set-primary-color pipe1 0.0 0.5 0.0 0.3)
(set-wind pipe1 2.0 0.0)

(set-primary-color (pipe-connector 84 23 15.5 0.6) 0.0 0.5 0.0 0.3)
 
;;SET NIGHT-TIME ON UPPER PLATFORMS.

(trigger 84 21 2.0 (lambda () (night)))

(diamond 84 20)



;; SET JUMP TO HIGH ON MOON;

(trigger 56 13 1.0 (lambda () (jump 1.6)))





;; SET TIME DAY ON MOON.

(trigger 40 13.5 2.0 (lambda () (day)))


;; SET JUMP TO HIGH;

(trigger 47 8 1.0 (lambda () (jump 1.6)))


;(define pipetemp3 
;(multi-pipe
;'((47.7 7.7 13.6)(44.5 8.0 11.5)
;  (44.5 18.0 -0.5)(40.5 18.0 -0.5)) 0.5 #t))

(define pipe17(pipe 47.2 7.2 13.6 44 7.5 11.5 0.5))
(set-primary-color pipe17 0.0 0.5 0.0 0.3)

(set-primary-color (pipe-connector 44 7.5 11.5 0.6) 0.0 0.5 0.0 0.3)

(define pipe18(pipe 44 7.5 11.5 44 17.5 -0.5 0.5))
(set-primary-color pipe18 0.0 0.5 0.0 0.3)

(set-primary-color (pipe-connector 44 17.5 -0.5 0.6) 0.0 0.5 0.0 0.3)

(define pipe19(pipe 44 17.5 -0.5 40.5 18.0 -0.5 0.5))
(set-primary-color pipe19 0.0 0.5 0.0 0.3)




;; SET DAY TIME AFTER SWITCH.

(trigger 40 17 2.0 (lambda () (day)))



;;Extra balls on Moon.

(add-modpill 44 17 *mod-extra-life* 30 -1)

(add-modpill 38 13 *mod-extra-life* 30 -1)



;; Moon Base to Exit.


(pipe-connector 35.5 22.9 -2.0 0.1)
(pipe 35.5 22.9 -2.0 38.9 18.5 -1.0 0.1) 
(pipe-connector 38.9 18.5 -1.0 0.1)

(pipe-connector 35.1 22.5 -2.0 0.1)
(pipe 35.1 22.5 -2.0 38.5 18.1 -1.0 0.1)
(pipe-connector 38.5 18.1 -1.0 0.1)

;; SET JUMP TO Low + Fog.

(trigger 35 23 0.5 (lambda () (jump 1.0)))
(trigger 34.5 23.5 1.0 (lambda () (fog)))

;;Star Ship - Save Place.

(diamond 31 37)


;;Extra LIfe at top of city

(add-modpill 63 64 *mod-extra-life* 60 -1)

;;back from top of city

(define pipee 
(multi-pipe
'((66 61 2.5) (66 60.5 1.0) 
 (67 67 -2.5) (72 67 -2.5)) 0.5 #t))


;;up to lift-off

(define pipef (pipe 81 33 2.5 81.1 31.2 2.5 0.5))
(pipe-connector 81.1 31 2.5 0.5)
(define pipeg (pipe 81.1 31 2.6 81.1 30 9.2 0.5))
(set-wind pipeg 9.0 9.0)


;;Last set of guides (Cargo Hold).

(ff-bridge-y 34.0 51.0 8.0)

;;Final door and switch.

(define doorff5 (forcefield 33.5 58.5 0.0 1.0 0.0 0.0 2.0 *ff-bounce*))
(switch 31.0 58 (lambda () (set-onoff doorff5 #t)) (lambda () (set-onoff doorff5 #f)))

;;End of level

(add-goal 34 60 #f "bx3")


