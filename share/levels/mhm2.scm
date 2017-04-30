;;; Madness

(set-track-name "Mayhem")
(set-author "Warren D. Ober")
(set-start-position 252 250) 
(start-time 180)
(restart-time 30)
(map-is-transparent #t)
;;Weather conditions.

(day)




;;Set ball velocity.

(define speed 0.3)
(cond
 ((= (difficulty) *easy*) (set! speed 0.20))
 ((= (difficulty) *normal*) (set! speed 0.30))
 ((= (difficulty) *hard*) (set! speed 0.40)))


;;pipe rails...

(pipe-connector 238.5 249.5 3.3 0.1)
(pipe 238.5 249.5 3.3 241.5 249.5 3.3 0.1)
(pipe-connector 241.5 249.5 3.3 0.1) 
(pipe 241.5 249.5 3.3 241.5 246.5 3.3 0.1)
(pipe-connector 241.5 246.5 3.3 0.1)

(pipe-connector 241.5 244.5 3.3 0.1)
(pipe 241.5 244.5 3.3 244.5 244.5 3.3 0.1)
(pipe-connector 244.5 244.5 3.3 0.1) 
(pipe 244.5 244.5 3.3 244.5 241.5 3.3 0.1)
(pipe-connector 244.5 241.5 3.3 0.1)

(pipe-connector 246.5 243.5 3.8 0.1)
(pipe 246.5 243.5 3.8 249.5 243.5 3.8 0.1)
(pipe-connector 249.5 243.5 3.8 0.1) 
(pipe 249.5 243.5 3.8 249.5 240.5 3.8 0.1)
(pipe-connector 249.5 240.5 3.8 0.1)



;;Some balls.

(define mr1 (new-mr-black 243.5 214.5))
(set-acceleration mr1 7.0)
(define mr1-animator
  (animator 
     1.0 0.0 1.0 0.0 1.0 *animator-bounce*
     (lambda (value1) (set-primary-color mr1 value1 value1 0.0)))
)

(define mr2 (new-mr-black 245.5 214.5))
(set-acceleration mr2 7.0)
(define mr2-animator
  (animator 
     1.0 0.0 1.0 0.0 1.0 *animator-bounce*
     (lambda (value2) (set-primary-color mr2 value2 0.0 value2)))
)

(define mr3 (new-mr-black 241.5 214.5))
(set-acceleration mr3 7.0)
(define mr3-animator
  (animator 
     1.0 0.0 1.0 0.0 1.0 *animator-bounce*
     (lambda (value3) (set-primary-color mr3 0.0 value3 value3)))
)


;;Pipes to get back to top of level.


;(multi-pipe
;'((219.5 226.0 -2.2)(219.5 225.0 -2.2)
;  (219.5 224.0 -7.2)(219.5 222.0 -7.2)) 0.5 #t)

(define pipe1(pipe 219.5 226.0 -2.2 219.5 225.0 -2.2 0.5))
(set-primary-color pipe1 0.0 0.0 0.0 1.0)
;(set-wind pipe1 0.0 0.0)

(set-primary-color (pipe-connector 219.5 225.0 -2.2 0.5) 0.0 0.0 0.0 1.0)

(define pipe2(pipe 219.5 225.0 -2.2 219.5 224.0 -7.2 0.5))
(set-primary-color pipe2 0.0 0.0 0.0 1.0)
(set-wind pipe2 -12.0 -6.0)

(set-primary-color (pipe-connector 219.5 224.0 -7.2 0.5) 0.0 0.0 0.0 1.0)

(define pipe3(pipe 219.5 224.0 -7.2 219.5 222.0 -7.2 0.5))
(set-primary-color pipe3 0.0 0.0 0.0 1.0)


;forcefield & switch.

(define doorff (forcefield 217.0 248.0 0.0 0.0 3.0 0.0 3.0 *ff-bounce*))
(switch 250.0 188.0 (lambda () (set-onoff doorff #t)) (lambda () (set-onoff doorff #f)))


(diamond 235.0 188.0)


(if (= (difficulty) *easy*)
  (diamond 208.0 249.0)          ;; Creates a savepoint  easy
  (add-flag 208 249 50 1 0.1)    ;; or just a flag...
)



;;HOTSPOTS

(smart-trigger 206.5 239.5 0.5  

   (lambda () (set-cell-heights 206 239 206 239 -2.1 -2.1 -2.1 -2.1)
   )
   (lambda () (set-cell-heights 206 239 206 239 -5.1 -5.1 -5.1 -5.1))  

)

(smart-trigger 208.5 235.5 0.5  

   (lambda () (set-cell-heights 208 235 208 235 -2.8 -2.8 -2.8 -2.8)
   )
   (lambda () (set-cell-heights 208 235 208 235 -5.8 -5.8 -5.8 -5.8))  

)



(smart-trigger 207.5 228.5 0.5  

   (lambda () (set-cell-heights 207 228 207 228 -4.2 -4.2 -4.2 -4.2)
   )
   (lambda () (set-cell-heights 207 228 207 228 -7.2 -7.2 -7.2 -7.2))  

)


(smart-trigger 206.5 222.5 0.5  

   (lambda () (set-cell-heights 206 222 206 222 -3.7 -3.7 -3.7 -3.7)
   )
   (lambda () (set-cell-heights 206 222 206 222 -6.7 -6.7 -6.7 -6.7))  

)


(smart-trigger 208.5 217.5 0.5  

   (lambda () (set-cell-heights 208 217 208 217 -2.6 -2.6 -2.6 -2.6)
   )
   (lambda () (set-cell-heights 208 217 208 217 -5.6 -5.6 -5.6 -5.6))  

)


(smart-trigger 210.5 211.5 0.5  

   (lambda () (set-cell-heights 210 211 210 211 -1.6 -1.6 -1.6 -1.6)
   )
   (lambda () (set-cell-heights 210 211 210 211 -4.6 -4.6 -4.6 -4.6))  

)

(smart-trigger 206.5 207.5 0.5  

   (lambda () (set-cell-heights 206 207 206 207 -0.8 -0.8 -0.8 -0.8)
   )
   (lambda () (set-cell-heights 206 207 206 207 -3.8 -3.8 -3.8 -3.8))  

)



(set-horizon (new-baby 239.5 228.5) 2.5)
(set-horizon (new-baby 239.5 227.5) 2.5)


(add-flag 198 200 100 1 0.1)
(add-flag 200 202 100 1 0.1)
(add-flag 198 202 100 1 0.1)
(add-flag 200 200 100 1 0.1)

(add-flag 194 200 100 1 0.1)
(add-flag 192 202 100 1 0.1)
(add-flag 194 202 100 1 0.1)
(add-flag 192 200 100 1 0.1)

(add-flag 188 200 100 1 0.1)
(add-flag 186 202 100 1 0.1)
(add-flag 188 202 100 1 0.1)
(add-flag 186 200 100 1 0.1)

(add-flag 182 200 100 1 0.1)
(add-flag 180 202 100 1 0.1)
(add-flag 182 202 100 1 0.1)
(add-flag 180 200 100 1 0.1)



(add-flag 204 194 100 1 0.1)
(add-flag 206 196 100 1 0.1)
(add-flag 204 196 100 1 0.1)
(add-flag 206 194 100 1 0.1)

(add-flag 198 194 100 1 0.1)
(add-flag 200 196 100 1 0.1)
(add-flag 198 196 100 1 0.1)
(add-flag 200 194 100 1 0.1)

(add-flag 194 194 100 1 0.1)
(add-flag 192 196 100 1 0.1)
(add-flag 194 196 100 1 0.1)
(add-flag 192 194 100 1 0.1)

(add-flag 188 194 100 1 0.1)
(add-flag 186 196 100 1 0.1)
(add-flag 188 196 100 1 0.1)
(add-flag 186 194 100 1 0.1)

(add-flag 182 194 100 1 0.1)
(add-flag 180 196 100 1 0.1)
(add-flag 182 196 100 1 0.1)
(add-flag 180 194 100 1 0.1)



(add-flag 204 188 100 1 0.1)
(add-flag 206 190 100 1 0.1)
(add-flag 204 190 100 1 0.1)
(add-flag 206 188 100 1 0.1)

(add-flag 198 188 100 1 0.1)
(add-flag 200 190 100 1 0.1)
(add-flag 198 190 100 1 0.1)
(add-flag 200 188 100 1 0.1)

(add-flag 194 188 100 1 0.1)
(add-flag 192 190 100 1 0.1)
(add-flag 194 190 100 1 0.1)
(add-flag 192 188 100 1 0.1)

(add-flag 188 188 100 1 0.1)
(add-flag 186 190 100 1 0.1)
(add-flag 188 190 100 1 0.1)
(add-flag 186 188 100 1 0.1)

(add-flag 182 188 100 1 0.1)
(add-flag 180 190 100 1 0.1)
(add-flag 182 190 100 1 0.1)
(add-flag 180 188 100 1 0.1)



(add-flag 204 182 100 1 0.1)
(add-flag 206 184 100 1 0.1)
(add-flag 204 184 100 1 0.1)
(add-flag 206 182 100 1 0.1)

(add-flag 198 182 100 1 0.1)
(add-flag 200 184 100 1 0.1)
(add-flag 198 184 100 1 0.1)
(add-flag 200 182 100 1 0.1)

(add-flag 194 182 100 1 0.1)
(add-flag 192 184 100 1 0.1)
(add-flag 194 184 100 1 0.1)
(add-flag 192 182 100 1 0.1)

(add-flag 188 182 100 1 0.1)
(add-flag 186 184 100 1 0.1)
(add-flag 188 184 100 1 0.1)
(add-flag 186 182 100 1 0.1)

(add-flag 182 182 100 1 0.1)
(add-flag 180 184 100 1 0.1)
(add-flag 182 184 100 1 0.1)
(add-flag 180 182 100 1 0.1)


(add-flag 204 176 100 1 0.1)
(add-flag 206 178 100 1 0.1)
(add-flag 204 178 100 1 0.1)
(add-flag 206 176 100 1 0.1)

(add-flag 198 176 100 1 0.1)
(add-flag 200 178 100 1 0.1)
(add-flag 198 178 100 1 0.1)
(add-flag 200 176 100 1 0.1)

(add-flag 194 176 100 1 0.1)
(add-flag 192 178 100 1 0.1)
(add-flag 194 178 100 1 0.1)
(add-flag 192 176 100 1 0.1)

(add-flag 188 176 100 1 0.1)
(add-flag 186 178 100 1 0.1)
(add-flag 188 178 100 1 0.1)
(add-flag 186 176 100 1 0.1)





;;End of level

(add-goal 181 177 #t "mhm3")


