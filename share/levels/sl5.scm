;; SIX LEVELS
;; Marble Mansion

(day)
(set-track-name (_ "The Marble Mansion"))
(set-author "Ulrik Enstad")
(set-start-position 95.5 77.5)
(start-time 1000)
(scale-oxygen 0.3)

;; the start
(add-cactus 76 133 0.3)
(add-modpill 74 96 *mod-spike* 45 45)

(pipe 76.1 133.0 -1.9 77.0 133.0 -1.9 0.4)
(pipe-connector 77.0 133.0 -1.9 0.4)
(pipe 77.0 133.0 -1.9 78.0 133.0 -1.6 0.4)
(set-wind (pipe-connector 78.0 133.0 -1.6 0.4) 5.0 5.0)
(set-wind (pipe 78.0 133.0 -1.6 78.1 133.0 2.8 0.4) 5.0 5.0)

;; forcefield blocked mansion gate
(pipe 89.0 113.0 -1.9 89.0 114.0 -1.9 1.0)
(define ff (forcefield 87.8 113.2 0.0 2.7 0.0 0.0 2.0 *ff-bounce*)) 

;; These guys gard the gate
(define mr-red1
 (set-primary-color
  (set-acceleration
   (set-modtime
    (new-mr-black 87 113)
   *mod-spike* -1.)
  2.3)
 0.8 0.0 0.0)
)
(define mr-red2
 (set-primary-color
  (set-acceleration
   (set-modtime
    (new-mr-black 91 113)
   *mod-spike* -1.)
  2.3)
 0.8 0.0 0.0)
)

;; first floor roof and hidden area
(add-cyclic-platform 89 119 89 119 6.5 9.6 1. .5)

(sign (_ "Pull twice") 1 20 -1 53.5 95.5)
(switch 54 96
 (lambda ()
  (set-cell-heights 86 119 86 119 5.1 5.1 5.1 5.1)
  (set-cell-heights 85 119 85 119 4.5 4.5 4.5 4.5)
  (set-cell-heights 85 118 85 118 4.0 4.0 4.0 4.0)
  (set-cell-heights 84 118 84 118 3.5 3.5 3.5 3.5)
  (set-cell-heights 84 119 84 119 3.0 3.0 3.0 3.0)
  (sign (_ "You may check out the roof again") 0.8 20.0 -1 54 96)
 )
 (lambda ()
  (day)
 )
)

(add-cyclic-platform 51 89 51 89 -12.3 -8.0 1. .6)

(pipe 75 87 -11.2 53 87 -11.2 0.4)

(define mr-blue
 (set-acceleration
  (set-modtime
   (set-primary-color
    (set-secondary-color
     (new-mr-black 46.5 84.5)
    1.0 1.0 1.0)
   0.0 0.3 1.0)
  *mod-speed* -1.)
 8.0)
)

(add-teleport 48 97 96 78 0.2)

(set-primary-color (add-flag 100 114 0 #t 0.1) 1.0 0.5 0.0)
(trigger-once 100 114 0.3 (lambda () (scale-oxygen 3.0)))
(sign (_ "Oxygen") 0.8 20.0 -1 100 114)

(define mr-grey
 (set-primary-color
  (set-acceleration
   (new-mr-black 91.5 115.5)
  1.6)
 0.5 0.5 0.5)
)

;; second floor roof
(add-flag 83 121 50 #t 0.1)
(define timeflag1 (set-primary-color (add-flag 82 121 0 #t 0.1) 0.0 1.0 0.0))
(time-on-death timeflag1 30)

(add-flag 80 128 50 #t 0.1)

(add-cyclic-platform 80 127 80 128 9.6 12.1 1. .6)

(add-modpill 79 135 *mod-extra-life* 10 0)

(diamond 89 120)

;; third floor roof

(define mr-red
 (set-primary-color
  (set-acceleration
   (set-modtime
    (new-mr-black 81.5 131.5)
   *mod-spike* -1.)
  2.3)
 0.8 0.0 0.0)
)

(add-cyclic-platform 92 127 92 127 12.1 15.2 1. .6)

(pipe 99.7 123.5 12.0 99.7 126.5 12.0 0.08)
(pipe 100.3 123.5 12.0 100.3 126.5 12.0 0.08)

(add-goal 99 135 #t "sl6")

;; hints to win level:
;; get spike modpill to kill cactus
;; grab oxygen flag to breath underwater in the pond outside the mansion
