import { vec2 } from "gl-matrix";

let tmp;

export class VecMath {
    static limit(vec: vec2, limit: number) {
        tmp = vec2.sqrLen(vec)
        if (tmp > limit * limit) {
            //setting to maximum limit if greater
            vec2.normalize(vec, vec);
            vec2.scale(vec, vec, limit);
        }
    }

    static setMag(vec: vec2, mag: number) {
        vec2.normalize(vec, vec);
        vec2.scale(vec, vec, mag);
    }
}