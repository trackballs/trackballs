;;; Four Doors

;; Environment Section. (defines Level settings)

(set-track-name "Four Doors")
(set-author "Warren D. Ober")
(set-start-position  14 2)     ;try these: 31 44)  82 24)  66 63)
(start-time 720)
(restart-time 90)
(map-is-transparent #t)

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


(define doorff1 (forcefield 29.0 29.0 0.0 5.0 0.0 0.0 6.0 *ff-bounce*))
(switch 15.5 54.5 (lambda () (set-onoff doorff1 #f)) (lambda () (set-onoff doorff1 #f)))

(define doorff2 (forcefield 29.0 31.0 0.0 5.0 0.0 0.0 6.0 *ff-bounce*))
(switch 31.5 84.5 (lambda () (set-onoff doorff2 #f)) (lambda () (set-onoff doorff2 #f)))

(define doorff3 (forcefield 29.0 33.0 0.0 5.0 0.0 0.0 6.0 *ff-bounce*))
(switch 68.5 42.5 (lambda () (set-onoff doorff3 #f)) (lambda () (set-onoff doorff3 #f)))

(define doorff4 (forcefield 29.0 35.0 0.0 5.0 0.0 0.0 6.0 *ff-bounce*))
(switch 48.5 5.5 (lambda () (set-onoff doorff4 #f)) (lambda () (set-onoff doorff4 #f)))


;;Flags near start.

(add-flag 14 18 350 1 0.1)
(add-flag 14 26 150 1 0.1)
(add-flag 17 26 350 1 0.1)
(add-flag 16 40 200 1 0.1)

;;Flags by spikes.

(add-flag 9 48 150 1 0.1)
(add-flag 9 51 250 1 0.1)
(add-flag 9 55 150 1 0.1)
(add-flag 9 57 350 1 0.1)

;;Flags near garden.

(add-flag 15 64 350 1 0.1)
(add-flag 17 60 250 1 0.1)
(add-flag 19 66 250 1 0.1)
(add-flag 16 69 150 1 0.1)
(add-flag 13 69 350 1 0.1)
(add-flag 13 67 150 1 0.1)
(add-flag 12 62 350 1 0.1)

;;Flags in park.

(add-flag 22 88 350 1 0.1)
(add-flag 21 90 250 1 0.1)
(add-flag 33 83 250 1 0.1)
(add-flag 34 84 150 1 0.1)

;;Flags in city.

(add-flag 61 63 350 1 0.1)
(add-flag 66 56 150 1 0.1)
(add-flag 83 28 350 1 0.1)
(add-flag 81 37 350 1 0.1)
(add-flag 78 37 250 1 0.1)
(add-flag 78 40 250 1 0.1)
(add-flag 80 40 150 1 0.1)
(add-flag 80 44 350 1 0.1)
(add-flag 81 50 250 1 0.1)
(add-flag 81 54 250 1 0.1)
(add-flag 76 55 150 1 0.1)
(add-flag 66 62 350 1 0.1)
(add-flag 66 66 250 1 0.1)

;;Flags on Moon.

(add-flag 35 14 350 1 0.1)
(add-flag 39 8 350 1 0.1)
(add-flag 44 15 350 1 0.1)


;;Flags in pyramids.

(add-flag 31 41 1000 1 0.1)
(add-flag 31 44 1000 1 0.1)

;;Spikes of doom

(add-sidespike 11.5 49.5 speed 0.1 1)
(add-sidespike 11.5 53.5 speed 0.1 4)
(add-sidespike 12.5 49.5 speed 0.2 4)
(add-sidespike 12.5 51.5 speed 0.2 4)
(add-sidespike 14.5 53.5 speed 0.1 4)
(add-sidespike 14.5 52.5 speed 0.1 2)
(add-sidespike 15.5 50.5 speed 0.1 1)
(add-spike 13.5 49.5 speed -0.00)
(add-spike 13.5 50.5 speed -0.20)
(add-spike 13.5 51.5 speed -0.40)
(add-spike 13.5 52.5 speed -0.60)
(add-spike 10.5 50.5 speed -0.00)
(add-spike 11.5 50.5 speed -0.20)
(add-spike 11.5 51.5 speed -0.40)
(add-spike 11.5 52.5 speed -0.60)
(add-spike 12.5 53.5 speed -0.80)
(add-spike 12.5 54.5 speed -0.00)
(add-spike 13.5 54.5 speed -0.00)
(add-spike 13.5 53.5 speed -0.20)
(add-spike 15.5 53.5 speed -0.60)
(add-spike 14.5 49.5 speed -0.60)
(add-spike 15.5 49.5 speed -0.80)
(add-spike 14.5 51.5 speed -0.00)
(add-spike 15.5 51.5 speed -0.00)

;;Garden Platforms.  x0  y0  x1  y1   Low  Hi  Offset Speed.

(add-cyclic-platform 12  70  12  70  -6.0 -3.5  1.0   0.8)
(add-cyclic-platform 15  69  15  69  -6.0 -3.5  1.5   0.6)
(add-cyclic-platform 18  67  18  67  -6.0 -3.5  2.0   0.7)
(add-cyclic-platform 14  62  14  62  -6.0 -3.5  1.0   1.0)
(add-cyclic-platform 17  62  17  62  -6.0 -3.5  1.5   0.9)


;;These are the guides in the city

(forcefield 68.5 52.1 0.2 7.0 0.0 0.0 0.1 *ff-bounce*)
(forcefield 68.1 52.5 -0.1 7.8 0.0 0.0 0.1 *ff-bounce*)
(forcefield 68.5 52.9 0.2 7.0 0.0 0.0 0.1 *ff-bounce*)

(forcefield 76.1 50.5 0.2 0.0 -10.0 0.0 0.1 *ff-bounce*)
(forcefield 76.5 50.9 -0.1 0.0 -10.8 0.0 0.1 *ff-bounce*)
(forcefield 76.9 50.5 0.2 0.0 -10.0 0.0 0.1 *ff-bounce*)

(forcefield 68.5 38.1 0.2 7.0 0.0 0.0 0.1 *ff-bounce*)
(forcefield 68.1 38.5 -0.1 7.8 0.0 0.0 0.1 *ff-bounce*)
(forcefield 68.5 38.9 0.2 7.0 0.0 0.0 0.1 *ff-bounce*)

(forcefield 66.1 37.5 0.2 0.0 -10.0 0.0 0.1 *ff-bounce*)
(forcefield 66.5 37.9 -0.1 0.0 -10.8 0.0 0.1 *ff-bounce*)
(forcefield 66.9 37.5 0.2 0.0 -10.0 0.0 0.1 *ff-bounce*)

(forcefield 67.5 23.1 0.2 10.0 0.0 0.0 0.1 *ff-bounce*)
(forcefield 67.1 23.5 -0.1 10.8 0.0 0.0 0.1 *ff-bounce*)
(forcefield 67.5 23.9 0.2 10.0 0.0 0.0 0.1 *ff-bounce*)

;;2nd set of guides (It is possible!)

(forcefield 67.5 25.1 0.2 14.0 0.0 0.0 0.1 *ff-bounce*)
(forcefield 67.1 25.5 -0.1 14.8 0.0 0.0 0.1 *ff-bounce*)
(forcefield 67.5 25.9 0.2 14.0 0.0 0.0 0.1 *ff-bounce*)

(forcefield 64.1 41.5 0.2 0.0 -17.0 0.0 0.1 *ff-bounce*)
(forcefield 64.5 41.9 -0.1 0.0 -17.8 0.0 0.1 *ff-bounce*)
(forcefield 64.9 41.5 0.2 0.0 -17.0 0.0 0.1 *ff-bounce*)

(forcefield 64.1 68.5 0.2 0.0 -26.0 0.0 0.1 *ff-bounce*)
(forcefield 64.5 68.9 -0.1 0.0 -26.8 0.0 0.1 *ff-bounce*)
(forcefield 64.9 68.5 0.2 0.0 -26.0 0.0 0.1 *ff-bounce*)


;;PIPES GROUP 1 (from first raised area to second)

(define pipe2(pipe 84.5 13.5 14.0 84.5 10.6 12.5 0.5))
(set-primary-color pipe2 0.0 0.5 0.0 0.3)

(set-primary-color (pipe-connector 84.5 10.5 12.5 0.6) 0.0 0.5 0.0 0.3)

(define pipe3(pipe 84.5 10.5 12.5 84.5 7.6 12.5 0.5))
(set-primary-color pipe3 0.0 0.5 0.0 0.3)
 
(define pipe4(pipe 84.5 7.5 12.5 80.5 7.6 12.5 0.5))
(set-primary-color pipe4 0.0 0.5 0.0 0.3)

(set-primary-color (pipe-connector 84.5 7.5 12.5 0.6) 0.0 0.5 0.0 0.3)

(define pipe5(pipe 80.5 7.6 12.5 77.5 10.5 12.5 0.5))
(set-primary-color pipe5 0.0 0.5 0.0 0.3)

(set-primary-color (pipe-connector 80.5 7.5 12.5 0.6) 0.0 0.5 0.0 0.3)


;;PIPES GROUP 2 (from second raised area to lower area)

(define pipe6(pipe 73.5 10.0 12.3 73.5 5.2 9.7 0.5))
(set-primary-color pipe6 0.0 0.5 0.0 0.3)

(set-primary-color (pipe-connector 73.5 5.1 9.6 0.6) 0.0 0.5 0.0 0.3)

(define pipe11(pipe 73.5 5.1 9.5 73.5 8.5 7.5 0.5))
(set-primary-color pipe11 0.0 0.5 0.0 0.3)

(set-primary-color (pipe-connector 73.5 8.5 7.5 0.6) 0.0 0.5 0.0 0.3)

(define pipe12(pipe 73.5 8.5 7.5 70.5 13.5 9.0 0.5))
(set-primary-color pipe12 0.0 0.5 0.0 0.3)

(set-primary-color (pipe-connector 70.5 13.5 9.0 0.6) 0.0 0.5 0.0 0.3)

(define pipe13(pipe 70.5 13.5 9.0 65.5 13.5 9.0 0.5))
(set-primary-color pipe13 0.0 0.5 0.0 0.3)


;;PIPES GROUP 3 (from second raised area to upper area)

(define pipe7(pipe 70.5 12.5 12.5 66.6 12.5 12.5 0.5))
(set-primary-color pipe7 0.0 0.5 0.0 0.3)

(define pipe8(pipe 66.4 12.4 12.5 60.7 15.5 14.0 0.5))
(set-primary-color pipe8 0.0 0.5 0.0 0.3)

(set-primary-color (pipe-connector 66.4 12.4 12.5 0.6) 0.0 0.5 0.0 0.3)

(define pipe9(pipe 60.6 15.5 14.0 56.5 10.5 14.0 0.5))
(set-primary-color pipe9 0.0 0.5 0.0 0.3)

(set-primary-color (pipe-connector 60.5 15.5 14.0 0.6) 0.0 0.5 0.0 0.3)

(define pipe10(pipe 56.5 10.5 14.0 51.0 6.5 14.5 0.5))
(set-primary-color pipe10 0.0 0.5 0.0 0.3)

;Pipes group 4 (from upper raised area to moon.)

(define pipe14(pipe 55.6 13.7 5.7 51.5 13.7 3.5 0.5))
(set-primary-color pipe14 0.0 0.5 0.0 0.3)

(set-primary-color (pipe-connector 51.5 13.7 3.5 0.6) 0.0 0.5 0.0 0.3)

(define pipe15(pipe 51.5 13.7 3.5 45.0 14.0 -3.5 0.5))
(set-primary-color pipe15 0.0 0.5 0.0 0.3)

(set-primary-color (pipe-connector 45.0 14.0 -3.5 0.6) 0.0 0.5 0.0 0.3)

(define pipe16(pipe 45.0 14.0 -3.5 41.0 14.0 -3.5 0.5))
(set-primary-color pipe16 0.0 0.5 0.0 0.3)



;;Bumpers in Starship Green-rooms.
   
(forcefield 30.0 46.0 0.2  3.0 0.0 0.0 0.1 *ff-bounce*)
(forcefield 33.0 46.0 -3.3  0.0 -3.0 0.0 0.1 *ff-bounce*)
(forcefield 33.0 43.0 0.2 -3.0 0.0 0.0 0.1 *ff-bounce*)
(forcefield 30.0 43.0 0.2  0.0 3.0 0.0 0.1 *ff-bounce*)

(forcefield 30.0 42.5 0.2  3.0 0.0 0.0 0.1 *ff-bounce*)
(forcefield 33.0 42.5 0.2  0.0 -3.0 0.0 0.1 *ff-bounce*)
(forcefield 33.0 39.5 0.2 -3.0 0.0 0.0 0.1 *ff-bounce*)
(forcefield 30.0 39.5 0.2  0.0 3.0 0.0 0.1 *ff-bounce*)


;;Pyramids of forcefields (Greenrooms).

;(forcefield rX   rY    rZ rDx  rDy rDz rHeight “flag”)

(forcefield 29.5 46.5 -3.5 2.0 -2.0 3.5 3.5 *ff-bounce*)
(forcefield 33.5 46.5 -7.0 -2.0 -2.0 3.5 3.5 *ff-bounce*)
(forcefield 33.5 42.5 -3.5 -2.0 2.0 3.5 3.5 *ff-bounce*)
(forcefield 29.5 42.5 -3.5 2.0 2.0 3.5 3.5 *ff-bounce*)

(forcefield 29.5 43.0 -3.5 2.0 -2.0 3.5 3.5 *ff-bounce*)
(forcefield 33.5 43.0 -3.5 -2.0 -2.0 3.5 3.5 *ff-bounce*)
(forcefield 33.5 39.0 -3.5 -2.0 2.0 3.5 3.5 *ff-bounce*)
(forcefield 29.5 39.0 -3.5 2.0 2.0 3.5 3.5 *ff-bounce*)



;;Procedure Section. (Gameplay starts.)


(set-primary-color (sign "4 hidden switches open exit doors." 0.8 0.0 -0.1 14.5 22.5) 0.2 0.5 1.0 1.0)

(set-primary-color (sign "A switch is nearby." 0.8 0.0 -0.1 13.5 48.5) 0.2 0.5 1.0 1.0)

(set-primary-color (sign "Peace." 0.8 0.0 -0.1 15.5 66.5) 0.2 0.5 1.0 1.0)
 

;;Extra life near start.

(add-modpill 18.5 34.5 *mod-extra-life* 30 -1)


;;Made it to the Garden! Save your place.


(diamond 9.5 59.5)
(trigger 9.0 59.0 1.0 (lambda () (fog)))
(trigger 9.7 59.7 1.0 (lambda () (jump 1.0)))


;;A bird flys by in Park.

(add-bird 11 95 50 60 0.5 2)


;;A floater for the pond jump.

(add-modpill 23.5 81.5 *mod-float* 60 -1)


;;This is the water pipe.

(pipe 31.5 81.5 -7.5 27.5 81.5 -7.5 1.0)


;;Mr. Black in Pipe

(define mr-black1 (new-mr-black 28.5 81.5))
(set-acceleration mr-black1 2.0)


;;Extra life Near Switch.

(add-modpill 35.5 81.5 *mod-extra-life* 30 -1)



;;Welcome to the city. Save your place here.

(diamond 60.5 70.5)
(trigger 60.0 70.0 1.0 (lambda () (fog)))
(trigger 60.3 70.3 1.0 (lambda () (jump 1.0)))


;;Made it through guides, save your place again.

(diamond 80.5 24.5)
(trigger 80.0 24.0 1.0 (lambda () (fog)))
(trigger 80.7 24.7 1.0 (lambda () (jump 1.0)))

;;Up to next level

(add-cyclic-platform 
 ;;; Positions
80 21 81 22 
 ;;; Low, High, Time Offset, Speed
 -3. 7. 1. 1.0)



;;Extra life By Platform.

(add-modpill 82.5 27.5 *mod-extra-life* 30 -1)



;;BEGIN LIFT SHUTE

(define shute(pipe 84.5 24.5 7.5 84.5 23.6 15.4 0.5))
(set-primary-color shute 0.0 0.5 0.0 0.3)
(set-wind shute 9.0 0.0)

(define pipe1(pipe 84.5 23.4 15.6 84.5 21.3 15.5 0.5))
(set-primary-color pipe1 0.0 0.5 0.0 0.3)
(set-wind pipe1 2.0 0.0)

(set-primary-color (pipe-connector 84.5 23.5 15.5 0.6) 0.0 0.5 0.0 0.3)
 
;;SET NIGHT-TIME ON UPPER PLATFORMS.

(trigger 84.5 21.5 2.0 (lambda () (night)))

(diamond 84.5 20.5)



;; SET JUMP TO HIGH ON MOON;

(trigger 56.5 13.5 1.0 (lambda () (jump 1.6)))





;; SET TIME DAY ON MOON.

(trigger 40.5 14.0 2.0 (lambda () (day)))


;; SET JUMP TO HIGH;

(trigger 47.5 8.5 1.0 (lambda () (jump 1.6)))


;(define pipetemp3 
;(multi-pipe
;'((47.7 7.7 13.6)(44.5 8.0 11.5)
;  (44.5 18.0 -0.5)(40.5 18.0 -0.5)) 0.5 #t))

(define pipe17(pipe 47.7 7.7 13.6 44.5 8.0 11.5 0.5))
(set-primary-color pipe17 0.0 0.5 0.0 0.3)

(set-primary-color (pipe-connector 44.5 8.0 11.5 0.6) 0.0 0.5 0.0 0.3)

(define pipe18(pipe 44.5 8.0 11.5 44.5 18.0 -0.5 0.5))
(set-primary-color pipe18 0.0 0.5 0.0 0.3)

(set-primary-color (pipe-connector 44.5 18.0 -0.5 0.6) 0.0 0.5 0.0 0.3)

(define pipe19(pipe 44.5 18.0 -0.5 40.5 18.0 -0.5 0.5))
(set-primary-color pipe19 0.0 0.5 0.0 0.3)




;; SET DAY TIME AFTER SWITCH.

(trigger 40.5 17.5 2.0 (lambda () (day)))



;;Extra balls on Moon.

(add-modpill 44.5 17.5 *mod-extra-life* 30 -1)

(add-modpill 38.5 13.5 *mod-extra-life* 30 -1)



;; Moon Base to Exit.


(pipe-connector 36.0 23.4 -2.0 0.1)
(pipe 36.0 23.4 -2.0 39.4 19.0 -1.0 0.1) 
(pipe-connector 39.4 19.0 -1.0 0.1)

(pipe-connector 35.6 23.0 -2.0 0.1)
(pipe 35.6 23.0 -2.0 39.0 18.6 -1.0 0.1)
(pipe-connector 39.0 18.6 -1.0 0.1)

;; SET JUMP TO Low + Fog.

(trigger 35.5 23.5 0.5 (lambda () (jump 1.0)))
(trigger 35.0 24.0 1.0 (lambda () (fog)))

;;Star Ship - Save Place.

(diamond 31.5 37.5)


;;Extra LIfe at top of city

(add-modpill 63.5 64.5 *mod-extra-life* 60 -1)

;;back from top of city

(define pipee 
(multi-pipe
'((66.5 61.5 2.5) (66.5 61.0 1.0) 
 (67.5 67.5 -2.5) (72.5 67.5 -2.5)) 0.5 #t))


;;up to lift-off

(define pipef (pipe 81.5 33.5 2.5 81.5 31.7 2.5 0.5))
(pipe-connector 81.5 31.5 2.5 0.5)
(define pipeg (pipe 81.5 31.5 2.6 81.5 30.5 9.2 0.5))
(set-wind pipeg 2.2 0.0)


;;Last set of guides (Cargo Hold).

(forcefield 34.1 51.5 0.2 0.0 8.0 0.0 0.1 *ff-nothing*)
(forcefield 34.5 51.1 -0.1 0.0 8.8 0.0 0.1 *ff-bounce*)
(forcefield 34.9 51.5 0.2 0.0 8.0 0.0 0.1 *ff-bounce*)

;;Final door and switch.

(define doorff5 (forcefield 34.0 59.0 0.0 1.0 0.0 0.0 2.0 *ff-bounce*))
(switch 31.5 58.5 (lambda () (set-onoff doorff5 #t)) (lambda () (set-onoff doorff5 #f)))

;;End of level

(add-goal 34 60 #f "bx3")


