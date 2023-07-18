import React from 'react';
import { Demo } from './demo';

export const Home = () => {
  return (
    <div
      style={{
        width: '100%',
        display: 'flex',
        flexDirection: 'column',
        alignItems: 'center',
        overflow: 'hidden',
      }}
    >
      <Demo />
      <div
        style={{
          width: 'min(800px, 100%)',
          margin: 'auto',
        }}
      >
        <h1>Otto Simulator</h1>
        <p>Lorem ipsum dolor, sit amet consectetur adipisicing elit. Quam quasi sunt ex incidunt unde necessitatibus fugiat ipsum dicta dolorem nihil. Ipsum dicta itaque dignissimos. Dicta ea quod asperiores voluptatibus laborum?</p>
      </div>
    </div>
  );
};
