import p5 from "p5";

interface Point {
    x: number;
    y: number;
}

export class Boundry<T extends Point> {
    constructor(
        readonly x: number,
        readonly y: number, 
        readonly w: number, 
        readonly h: number){

    }

    contains(point: T) {
        if (this.x > point.x || this.y > point.y) {
            return false;
        }

        if (this.x + this.w < point.x || this.y + this.h < point.y) {
            return false;
        }

        return true;
    }

    intersects(boundry: Boundry<T>) {
        if(boundry.x + boundry.w <= this.x || 
            boundry.y + boundry.h <= this.y || 
            boundry.x >= this.x + this.w || 
            boundry.y >= this.y + this.h) {
               return false;
           }

        return true;
    }

    show(p: p5, distinct = false) {
        p.stroke(255, 10);
        p.rect(this.x, this.y, this.w, this.h);
    }
}

export class Quadtree<T extends Point>{
    capacity = 4;
    points: {x: number, y: number}[] = [];
    divided = false;
    pointsCount = 0;
    area = 0;
    boundry!: Boundry<T>;
    nw!: Quadtree<T>;
    ne!: Quadtree<T>;
    sw!: Quadtree<T>;
    se!: Quadtree<T>;

    constructor(readonly p: p5){}

    init(x: number, y: number, w: number, h: number) {
        this.boundry = new Boundry(x, y, w, h);
        this.area = w * h;
    }

    insert(point: T) {
        if (!this.boundry.contains(point)) {
            return false;
        }

        this.pointsCount++;

        if (this.points.length < this.capacity){
            this.points.push(point);
            return true;
        } else {
            if (!this.divided) {
                this.subdivide();
            } 

            if (this.nw.insert(point)){}
            else if (this.ne.insert(point)){}
            else if (this.sw.insert(point)){}
            else if (this.se.insert(point)){}
            return true;
        }
    }

    subdivide(){
        let x = this.boundry.x;
        let y = this.boundry.y;
        let w = this.boundry.w;
        let h = this.boundry.h;

        this.nw = new Quadtree(this.p);
        this.nw.init(x, y, w / 2, h / 2 )
        this.ne = new Quadtree(this.p);
        this.ne.init(x + w / 2, y, w / 2, h / 2 )
        this.sw = new Quadtree(this.p);
        this.sw.init(x, y + h / 2, w / 2, h / 2 )
        this.se = new Quadtree(this.p);
        this.se.init(x + w / 2, y + h / 2, w / 2, h / 2 )
        this.divided = true;
    }

    query(range: Boundry<T>, found: T[] = []) {
        if (!found){
            found = [];
        }

        if (!this.boundry.intersects(range)) {
            return found;
        } else {
            for (let p of this.points) {
                if (range.contains(p as T)) {
                    found.push(p as T);
                }
            }

            if (this.divided) {
                this.nw.query(range, found);
                this.ne.query(range, found);
                this.sw.query(range, found);
                this.se.query(range, found);
            }

        }

        return found;
    }

    dispose() {
        if (this.divided) {
            (this.nw as any) = null;
            (this.sw as any) = null;
            (this.ne as any) = null;
            (this.se as any) = null;
            this.divided = false;
        }
        this.points = [];
    }

    reset() {
        this.dispose();
        this.pointsCount = 0;
    }

    show(){
        this.boundry.show(this.p);
        if (this.divided){
            this.nw.show();
            this.ne.show();
            this.sw.show();
            this.se.show();
        }
    }
}