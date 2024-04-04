/*!
 * @author Vojtěch Tomas <tomas@vojtatom.cz>
 * date 12/22/2018
 * DNA class for flocing simulation. 
 */

import p5 from "p5";


/**
 *Class DNA represents agent's DNA. DNA is stored as a single array and 
 *each property is accesible as a proeprty of each DNA instance.
 *
 *DNA structure:
 *  [] maximal force
 *  [] maximal speed
 *  [] moodiness
 *  [] mental endurance
 *  [] meantal health
 *  [] food perception radius
 *  [] alignment perception radius
 *  [] cohesion perception radius
 *  [] seperation perception radius
 *  [] view perception radius
 *  [] uncomfortable radius
 *  [] happy radius
 *  [] mood exchange radius
 * @class DNA
 */

const generalIndex = 0;
const generalCount = 6;
const zoneIndex = 6;
const zoneCount = 7;

export class DNA {

    sequence: number[];

    constructor(readonly p: p5){
        this.sequence = [ 
            /* general */ Math.random() * 0.6, Math.random() + 1.5, Math.random() * 0.1 + 0.001, Math.random() * 2 + 1, Math.random() * 20 - 10, 10,
            ///* general */ 0.2, 2.0, 0.001, 2 + 1, 20 - 10, 10,
            /* zones */   200, 40, 40, 40, 40, 20, 40, 30 
        ];
    }

    get force(){
        return this.sequence[generalIndex + 0];
    }

    get speed(){
        return this.sequence[generalIndex + 1];
    }

    get moodiness(){
        return this.sequence[generalIndex + 2];
    }

    get mentalEndurance(){
        return this.sequence[generalIndex + 3];
    }

    get mentalHealth(){
        return this.sequence[generalIndex + 4];
    }

    get moodLimit(){
        return this.sequence[generalIndex + 5];
    }

    get foodZone(){
        return this.sequence[zoneIndex + 0];
    }

    get alignmentZone(){
        return this.sequence[zoneIndex + 1];
    }

    get cohesionZone(){
        return this.sequence[zoneIndex + 2];
    }

    get seperationZone(){
        return this.sequence[zoneIndex + 3];
    }

    get viewZone(){
        return this.sequence[zoneIndex + 4];
    }

    get closeZone(){
        return this.sequence[zoneIndex + 5];
    }

    get happyZone() {
        return this.sequence[zoneIndex + 6];
    }

    get sensitiveZone() {
        return this.sequence[zoneIndex + 7];
    }

    get maxZone() {
        let m = 0;
        for (let i = zoneIndex + 1; i <= zoneIndex + zoneCount; i++)
            m = Math.max(m, this.sequence[i]);

        return m;
    }
}