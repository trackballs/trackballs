;;; Spyballs

(day)
(set-track-name (_ "Spyballs"))
(set-author "Warren D Ober")
(start-time 300)
(restart-time 15)

(set-start-position 174 149)   


;;Descent

(add-modpill 172 149 *mod-speed* 50 -20)

(smart-trigger 171 149 0.9
   (lambda () 
   (set-cell-flag 167 118 168 152  *cell-kill* #t)
   (set-cell-flag 165 118 166 152  *cell-kill* #t)
   )
   (lambda () 
   (set-cell-flag 167 118 168 152  *cell-kill* #f)
   (set-cell-flag 165 118 166 152  *cell-kill* #f)
   )  
)


;; 5th floor

(add-flag 167 129 100 #t 0.1)

(smart-trigger 167 125 0.9  
   (lambda () (set-cell-heights 169 124 169 124 17.0 17.0 17.0 17.0)
   (new-mr-black 168 124)
   )
   (lambda () (set-cell-heights 169 124 169 124 21.4 21.4 21 21))
)
(add-flag 167 124 100 #t 0.1)



;; 4th floor


(smart-trigger 165 122 0.9
   (lambda () (set-cell-heights 167 124 167 124 13.0 13.0 13.0 13.0)
   (new-mr-black 166 124)
   )
   (lambda () (set-cell-heights 167 124 167 124 17.0 17.0 17.0 17.0)
   )  
)
(add-flag 165 124 100 #t 0.1)


(add-flag 165 129 100 #t 0.1)


(smart-trigger 165 134 0.9  
   (lambda () (set-cell-heights 167 134 167 134 13 13 13 13)
   (set-modtime (set-modtime (new-mr-black 166 134) *mod-small* -1.) *mod-spike* -1.)
   )
   (lambda () (set-cell-heights 167 134 167 134 17.0 17.0 17.0 17.0))  
)
(add-flag 165 134 100 #t 0.1)


(smart-trigger 165 146 0.9  
   (lambda () (set-cell-heights 167 144 167 144 13.0 13.0 13.0 13.0)
   (new-mr-black 166 144)
   )
   (lambda () (set-cell-heights 167 144 167 144 17.0 17.0 17.0 17.0))  
)
(add-flag 165 144 100 #t 0.1)


(diamond 166 148)

;; 3rd floor

;clears upper floors.
(smart-trigger 164 149 0.9
   (lambda () 
   (set-cell-flag 163 118 164 145  *cell-kill* #t)
   (set-cell-flag 161 118 162 152  *cell-kill* #t)
   )
   (lambda () 
   (set-cell-flag 163 118 164 145  *cell-kill* #f)
   (set-cell-flag 161 118 162 152  *cell-kill* #f)
   )  
)


(add-flag 163 124 100 #t 0.1)


(smart-trigger 163 128 0.9  
   (lambda () (set-cell-heights 165 129 165 129 9.0 9.0 9.0 9.0)
   (new-mr-black 164 129)
   )
   (lambda () (set-cell-heights 165 129 165 129 13. 13. 13. 13.)
   )  
)
(add-flag 163 129 100 #t 0.1)


(smart-trigger 163 139 0.9  
   (lambda () (set-cell-heights 165 139 165 139 9.0 9.0 9.0 9.0)
   (new-mr-black 164 139)
   )
   (lambda () (set-cell-heights 165 139 165 139 13. 13. 13. 13.)
   )  
)
(add-flag 163 139 100 #t 0.1)

(add-flag 163 144 100 #t 0.1)

(diamond 164 122)



;; 2nd Floor.


(smart-trigger 161 124 0.9  
   (lambda () (set-cell-heights 163 124 163 124 5.0 5.0 5.0 5.0)
   (new-mr-black 162 124)
   )
   (lambda () (set-cell-heights 163 124 163 124 9.0 9.0 9.0 9.0)
   )  
)
(add-flag 161 124 100 #t 0.1)


(smart-trigger 161 133 0.9  
   (lambda () (set-cell-heights 163 134 163 134 5.0 5.0 5.0 5.0)
   (new-mr-black 162 134)
   )
   (lambda () (set-cell-heights 163 134 163 134 9.0 9.0 9.0 9.0))  
)
(add-flag 161 134 100 #t 0.1)


(add-flag 161 139 100 #t 0.1)


(smart-trigger 161 142 0.9  
   (lambda () (set-cell-heights 163 144 163 144 5.0 5.0 5.0 5.0)
   (set-modtime (set-modtime (new-mr-black 162 144) *mod-small* -1.) *mod-spike* -1.)
   )
   (lambda () (set-cell-heights 163 144 163 144 9.0 9.0 9.0 9.0)
   )  
)
(add-flag 161 144 100 #t 0.1)

(diamond 162 148)



;; 1st Floor.


(add-flag 159 124 100 #t 0.1)


(smart-trigger 159 129 0.9  
   (lambda () (set-cell-heights 161 129 161 129 1.0 1.0 1.0 1.0)
   (new-mr-black 160 129)
   )
   (lambda () (set-cell-heights 161 129 161 129 5.0 5.0 5.0 5.0)
   )  
)
(add-flag 159 129 100 #t 0.1)


(smart-trigger 159 139 0.9  
   (lambda () (set-cell-heights 161 139 161 139 1.0 1.0 1.0 1.0)
   (new-mr-black 160 139)
   )
   (lambda () (set-cell-heights 161 139 161 139 5.0 5.0 5.0 5.0)
   )  
)
(add-flag 159 139 100 #t 0.1)

(diamond 160 122)





;; Ground Floor.


(add-flag 157 129 100 #t 0.1)


(smart-trigger 157 134 0.9  
   (lambda () (set-cell-heights 159 134 159 134 -3.0 -3.0 -3.0 -3.0)
   (set-modtime (set-modtime (new-mr-black 158 134) *mod-small* -1.) *mod-spike* -1.)
   )
   (lambda () (set-cell-heights 159 134 159 134 1.0 1.0 1.0 1.0)
   )  
)
(add-flag 157 134 100 #t 0.1)


(smart-trigger 157 144 0.9  
   (lambda () (set-cell-heights 159 144 159 144 -3.0 -3.0 -3.0 -3.0)
   (new-mr-black 158 144)
   )
   (lambda () (set-cell-heights 159 144 159 144 1.0 1.0 1.0 1.0)
   )  
)
(add-flag 157 144 100 #t 0.1)





;; Lower Floor.



(smart-trigger 153 142 0.9  
   (lambda () (set-cell-heights 155 144 155 144 -8.0 -8.0 -8.0 -8.0)
   (new-mr-black 154 144)
   )
   (lambda () (set-cell-heights 155 144 155 144 -3.0 -3.0 -3.0 -3.0)
   )
)
(add-flag 153 144 100 #t 0.1)


(add-flag 153 134 100 #t 0.1)


(smart-trigger 153 128 0.9  
   (lambda () (set-cell-heights 155 129 155 129 -8.0 -8.0 -8.0 -8.0)
   (new-mr-black 154 129)
   )
   (lambda () (set-cell-heights 155 129 155 129 -3.0 -3.0 -3.0 -3.0) 
   )
)
(add-flag 153 129 100 #t 0.1)



(add-cyclic-platform 
 ;;; Positions 
 163 151 166 152 
 ;;; Low, High, Time Offset, Speed
 9.0 13.0 1.8 0.9)

(add-cyclic-platform 
 ;;; Positions
 165 118 168 119
 ;;; Low, High, Time Offset, Speed
 13.0 17.0 2. 1.)

(add-cyclic-platform 
 ;;; Positions
 161 118 164 119
 ;;; Low, High, Time Offset, Speed
 5. 9. 1.6 1.1)

(add-cyclic-platform 
 ;;; Positions
 159 151 162 152
 ;;; Low, High, Time Offset, Speed
 1. 5. 1.8 1.)

(add-cyclic-platform 
 ;;; Positions
 157 118 160 119
 ;;; Low, High, Time Offset, Speed
 -3. 1. 1.4 0.9)



;********************


(add-modpill 151.5 116.5 *mod-extra-life* 30 -1)

(add-modpill 151.5 114.5 *mod-speed* 30 -1)

(add-modpill 151.5 118.5 *mod-speed* 30 -1)


(add-modpill 155.5 116.5 *mod-extra-life* 30 -1)

(add-modpill 155.5 81.5 *mod-speed* 30 -1)

(add-modpill 155.5 149.5 *mod-speed* 30 -1)


;*********************

(add-cyclic-platform 
 ;;; Positions
 159 81 162 82
 ;;; Low, High, Time Offset, Speed
 1. 5. 2.0 1.1)

(add-cyclic-platform 
 ;;; Positions
 157 114 160 115
 ;;; Low, High, Time Offset, Speed
 -3. 1. 1.6 1.2)

(add-cyclic-platform 
 ;;; Positions
 165 114 168 115
 ;;; Low, High, Time Offset, Speed
 13. 17. 1.8 .9)

(add-cyclic-platform 
 ;;; Positions
 161 114 164 115
 ;;; Low, High, Time Offset, Speed
 5. 9. 1.4 1.3)

(add-cyclic-platform 
 ;;; Positions 
 163 81 166 82 
 ;;; Low, High, Time Offset, Speed
 9.0 13.0 1.2 .7)





;;Ascent;



;;Lower Floor.



;(smart-trigger 153 84 0.9
;   (lambda () 
;   (set-cell-flag 153 81 154 115  *cell-kill* #t)
;   (set-cell-flag 157 81 158 115  *cell-kill* #t)
;   )
;   (lambda () 
;   (set-cell-flag 153 81 154 115  *cell-kill* #f)
;   (set-cell-flag 157 81 158 115  *cell-kill* #f)
;   )  
;)



(smart-trigger 153 106 0.9  
   (lambda () (set-cell-heights 155 104 155 104 -8.0 -8.0 -8.0 -8.0)
   (new-mr-black 154 104)
   )
   (lambda () (set-cell-heights 155 104 155 104 -3.0 -3.0 -3.0 -3.0)
   )
)
(add-flag 153 104 100 #t 0.1)


(smart-trigger 153 96 0.9  
   (lambda () (set-cell-heights 155 94 155 94 -8.0 -8.0 -8.0 -8.0)
   (new-mr-black 154 94)
   )
   (lambda () (set-cell-heights 155 94 155 94 -3.0 -3.0 -3.0 -3.0)
   )  
)
(add-flag 153 94 100 #t 0.1)

(add-flag 153 89 100 #t 0.1)

;; Ground Floor.



(smart-trigger 157 91 0.9  

   (lambda () (set-cell-heights 159 89 159 89 -3.0 -3.0 -3.0 -3.0)
   (set-modtime (set-modtime (new-mr-black 158 89) *mod-small* -1.) *mod-spike* -1.)
   )
   (lambda () (set-cell-heights 159 89 159 89 1.0 1.0 1.0 1.0)
   )  
)
(add-flag 157 89 100 #t 0.1)

(add-flag 157 94 100 #t 0.1)

(smart-trigger 157 101 0.9

   (lambda () (set-cell-heights 159 99 159 99 -3.0 -3.0 -3.0 -3.0)
   (new-mr-black 158 99)
   )
   (lambda () (set-cell-heights 159 99 159 99 1.0 1.0 1.0 1.0))  
)
(add-flag 157 99 100 #t 0.1)



(smart-trigger 157 111 0.9  

   (lambda () (set-cell-heights 159 109 159 109 -3.0 -3.0 -3.0 -3.0)
   (new-mr-black 158 109)
    )
   (lambda () (set-cell-heights 159 109 159 109 1.0 1.0 1.0 1.0)
    )
)
(add-flag 157 109 100 #t 0.1)



;; 1st Floor



(smart-trigger 159 106 0.9  
   (lambda () (set-cell-heights 161 104 161 104 1.0 1.0 1.0 1.0)
   (new-mr-black 160 104)
   )
   (lambda () (set-cell-heights 161 104 161 104 5.0 5.0 5.0 5.0)
   )
)
(add-flag 159 104 100 #t 0.1)



(smart-trigger 159 96 0.9  
   (lambda () (set-cell-heights 161 94 161 94 1.0 1.0 1.0 1.0)
   (new-mr-black 160 94)
   )
   (lambda () (set-cell-heights 161 94 161 94 5.0 5.0 5.0 5.0)
   )
)
(add-flag 159 94 100 #t 0.1)


(smart-trigger 159 87 0.9  
   (lambda () (set-cell-heights 161 89 161 89 1.0 1.0 1.0 1.0)
   (set-primary-color (set-modtime (set-modtime (new-mr-black 160 89) *mod-small* -1.) *mod-spike* -1.) 0.9 0. 0. 1.)
   )
   (lambda () (set-cell-heights 161 89 161 89 5.0 5.0 5.0 5.0)
   )  
)
(add-flag 159 89 100 #t 0.1)


(diamond 160 85)
;(smart-trigger 160 85 0.9
;   (lambda () 
;   (set-cell-flag 157 81 159 115  *cell-kill* #t)
;   (set-cell-flag 161 81 163 115  *cell-kill* #t)
;   )
;   (lambda () 
;   (set-cell-flag 157 81 159 115  *cell-kill* #f)
;   (set-cell-flag 161 81 163 115  *cell-kill* #f)
;   )  
;)




;; 2nd floor

(diamond 162 111)


(smart-trigger 161 107 0.9  
   (lambda () (set-cell-heights 163 109 163 109 5.0 5.0 5.0 5.0)
   (set-primary-color (set-modtime (set-modtime (new-mr-black 162 109) *mod-small* -1.) *mod-spike* -1.) 0.9 0. 0. 1.)
   )
   (lambda () (set-cell-heights 163 109 163 109 9.0 9.0 9.0 9.0)
   )  
)
(add-flag 161 109 100 #t 0.1)



(smart-trigger 161 102 0.9  

   (lambda () (set-cell-heights 163 104 163 104 5.0 5.0 5.0 5.0)
   (new-mr-black 162 104)
   )
   (lambda () (set-cell-heights 163 104 163 104 9.0 9.0 9.0 9.0)) 
)
(add-flag 161 104 100 #t 0.1)


(smart-trigger 161 93 0.9  
   (lambda () (set-cell-heights 163 94 163 94 5.0 5.0 5.0 5.0)
   (set-primary-color (set-modtime (set-modtime (new-mr-black 162 94) *mod-small* -1.) *mod-spike* -1.) 0.9 0. 0. 1.)
   )
   (lambda () (set-cell-heights 163 94 163 94 9.0 9.0 9.0 9.0)
   )  
)
(add-flag 161 94 100 #t 0.1)


(smart-trigger 161 91 0.9  

   (lambda () (set-cell-heights 163 89 163 89 5.0 5.0 5.0 5.0)
   (new-mr-black 162 89)
   )
   (lambda () (set-cell-heights 163 89 163 89 9.0 9.0 9.0 9.0)
   )  
)
(add-flag 161 89 100 #t 0.1)



;; 3rd floor


(smart-trigger 163 96 0.9  
   (lambda () (set-cell-heights 165 94 165 94 9.0 9.0 9.0 9.0)
   (new-mr-black 164 94)
   )
   (lambda () (set-cell-heights 165 94 165 94 13.0 13.0 13.0 13.0)
   )  

)
(add-flag 163 94 100 #t 0.1)



(smart-trigger 163 98 0.9  
   (lambda () (set-cell-heights 165 99 165 99 9.0 9.0 9.0 9.0)
   (set-primary-color (set-modtime (set-modtime (new-mr-black 164 99) *mod-small* -1.) *mod-spike* -1.) 0.9 0. 0. 1.)
   )
   (lambda () (set-cell-heights 165 99 165 99 13.0 13.0 13.0 13.0)
   )  
)
(add-flag 161 99 100 #t 0.1)



(smart-trigger 163 106 0.9  
   (lambda () (set-cell-heights 165 104 165 104 9.0 9.0 9.0 9.0)
   (new-mr-black 164 104)
   )
   (lambda () (set-cell-heights 165 104 165 104 13.0 13.0 13.0 13.0))  

)
(add-flag 163 104 100 #t 0.1)


(diamond 164 85)
;(smart-trigger 164 85 0.9
;   (lambda () 
;   (set-cell-flag 159 91 161 115  *cell-kill* #t)
;   (set-cell-flag 161 81 163 115  *cell-kill* #t)
;   )
;   (lambda () 
;   (set-cell-flag 159 91 161 115  *cell-kill* #f)
;   (set-cell-flag 161 81 163 115  *cell-kill* #f)
;   )  
;)


;; 4th floor.


(diamond 166 111)

(smart-trigger 165 91 0.9
   (lambda () (set-cell-heights 167 89 167 89 13.0 13.0 13.0 13.0)
   (new-mr-black 166 89)
   )
   (lambda () (set-cell-heights 167 89 167 89 17.0 17.0 17.0 17.0)
   )  
)
(add-flag 165 89 100 #t 0.1)



(smart-trigger 165 98 0.9  
   (lambda () (set-cell-heights 167 99 167 99 13.0 13.0 13.0 13.0)
   (set-primary-color (set-modtime (set-modtime (new-mr-black 166 99) *mod-small* -1.) *mod-spike* -1.) 0.9 0. 0. 1.)
   )
   (lambda () (set-cell-heights 167 99 167 99 17.0 17.0 17.0 17.0))  
)
(add-flag 165 99 100 #t 0.1)


(add-flag 165 104 100 #t 0.1)


(smart-trigger 165 107 0.9  
   (lambda () (set-cell-heights 167 109 167 109 13.0 13.0 13.0 13.0)
   (new-mr-black 166 109)
   )
   (lambda () (set-cell-heights 167 109 167 109 17.0 17.0 17.0 17.0)
   )  
)
(add-flag 165 109 100 #t 0.1)




;; 5th floor.


(add-flag 167 109 100 #t 0.1)


(smart-trigger 167 106 0.9  
   (lambda () (set-cell-heights 169 104 169 104 17.0 17.0 17.0 17.0)
   (set-modtime (set-modtime (new-mr-black 168 104) *mod-small* -1.) *mod-spike* -1.)
   )
   (lambda () (set-cell-heights 169 104 169 104 21.0 21.0 21.0 21.0))  
)
(add-flag 167 104 100 #t 0.1)


(smart-trigger 167 96 0.9  
   (lambda () (set-cell-heights 169 94 169 94 17.0 17.0 17.0 17.0)
   (set-modtime (set-modtime (new-mr-black 168 94) *mod-small* -1.) *mod-spike* -1.)
   )
   (lambda () (set-cell-heights 169 94 169 94 21.0 21.0 21.0 21.0))  
)
(add-flag 167 94 100 #t 0.1)


(add-flag 167 89 100 #t 0.1)



(smart-trigger 167 171 0.9  

   (lambda () (set-cell-heights 169 89 169 89 16.9 16.9 16.9 16.9)
   (new-mr-black 168 89)
    (set-cell-flag 165 81 166 115  *cell-kill* #t)
   )
   (lambda () (set-cell-heights 169 89 169 89 21.4 21.4 21.5 21.5)
    (set-cell-flag 165 81 166 115  *cell-kill* #t)
   )  
)




;; Goal!
(add-goal 168 83 #f "mhm2")




